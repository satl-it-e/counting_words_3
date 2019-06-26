#include <thread>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <map>
#include <unordered_map>
#include <string>
#include <chrono>
#include <vector>
#include <fstream>
#include <sstream>
#include <type_traits>
#include <typeinfo>
#include <mutex>

#include <boost/locale.hpp>
#include <boost/locale/boundary/segment.hpp>
#include <boost/locale/boundary/index.hpp>
#include <boost/filesystem.hpp>

#include <tbb/concurrent_queue.h>
#include <ctpl.h>

#include "time_measure.h"
#include "config.h"
#include "my_archive.h"


std::mutex mutex_map;

using namespace boost::locale::boundary;
using namespace boost::filesystem;


std::map<std::string, int> index_string(std::string &content, std::locale &loc){
    // fold case
    content = boost::locale::to_lower(content, loc);

    // words to map
    std::map<std::string, int> cut_words;
    ssegment_index map(word, content.begin(), content.end(), loc);
    map.rule(word_any);

    for (ssegment_index::iterator it = map.begin(), e = map.end(); it != e; ++it) {
        cut_words[*it]++;
    }

    return cut_words;
}

void index_thread(int id, tbb::concurrent_queue<std::string> &mq_str, tbb::concurrent_queue<std::map<std::string, int>> &mq_map,
        std::string &poisson_str, std::map<std::string, int> &poisson_map, std::locale &loc){

    std::string process_str;

    while(true){
        if (mq_str.try_pop(process_str)){
            if (process_str == poisson_str){
                break;
            }
            std::map<std::string, int> processed_map = index_string(process_str, loc);
            mq_map.push(processed_map);
        }
    }
    mq_str.push(poisson_str);
    mq_map.push(poisson_map);
}


std::map<std::string, int> merge_maps(std::vector< std::map<std::string, int>> &maps){
    std::map<std::string, int> new_map;
    for (auto &map: maps){
        for (auto &el: map){
            new_map[el.first] += map[el.first];
        }
    }
    return new_map;
}



void merge_thread(int id, tbb::concurrent_queue<std::map<std::string, int>> &mq_maps, std::map<std::string, int> &poisson_map,
        const int &producers_num, int &poisson_maps_num){
    std::map<std::string, int> process_map;
    std::vector<std::map<std::string, int> > maps_in_hand;

    while(true){
        if(mq_maps.try_pop(process_map)){
            if (process_map == poisson_map){
                // Checking if this is not false poisson pillow
                mutex_map.lock();
                poisson_maps_num++;

                if (poisson_maps_num >= producers_num) {
                    // Check one more time
                    while(mq_maps.try_pop(process_map)){
                        if (process_map == poisson_map){
                            break;
                        }
                        maps_in_hand.push_back(std::move(process_map));
                    }

                    std::map<std::string, int> processed_map = merge_maps(maps_in_hand);
                    mq_maps.push(processed_map);

                    for (int i = 0; i < poisson_maps_num; i++){
                        mq_maps.push(poisson_map);
                    }
                    poisson_maps_num = 0;
                    mutex_map.unlock();
                    break;
                }
                mutex_map.unlock();

            } else {
                maps_in_hand.push_back(std::move(process_map));
                if (maps_in_hand.size() >= 2) {
                    std::map<std::string, int> processed_map = merge_maps(maps_in_hand);
                    maps_in_hand.clear();
                    mq_maps.push(processed_map);
                }
            }
        }
    }
}

int read_entry(MyArchive &marc, tbb::concurrent_queue<std::string> &mq_str, path &x){
    // opening archive
    std::string filename = x.string(), content;
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    if (boost::filesystem::extension(filename) == ".zip"){
        if (marc.init(filename) != 0){
            std::cerr << "Something wrong with file: " << filename << std::endl;
            return -1;
        }
        while (marc.next_content_available()){
            content = marc.get_next_content();
            mq_str.push(content);
        }

    } else if (boost::filesystem::extension(filename) == ".txt"){
        std::ifstream in_f(filename);
        if (! in_f.is_open() || in_f.rdstate()){
            std::cerr << "Couldn't open the file. " << filename << std::endl;
            return -2;
        }
        std::stringstream ss;
        ss << in_f.rdbuf();
        content = ss.str();
        mq_str.push(content);
    }
#ifdef print_wrong_file_extensions
    else {
        std::cerr << "Wrong file extension for file: " << filename << std::endl;
    }
#endif
    return 0;
}


void process_deep(MyArchive &marc, tbb::concurrent_queue<std::string> &mq_str, path x){
    if (is_regular_file(x)){
        read_entry(marc, mq_str, x);
    }

    else if (is_directory(x))
    {
        for (directory_entry& y : directory_iterator(x)){
            process_deep(marc, mq_str, y.path());
        }
    }

    else {
        std::cerr << x << " exists, but is not a regular file or directory\n";
    }
}

int main()
{
    std::string conf_file_name = "config.dat";

    // config
    MyConfig mc;
    mc.load_configs_from_file(conf_file_name);
    if (mc.is_configured()) {
        std::cout << "Configurations loaded successfully.\n" << std::endl;
    } else { std::cerr << "Error. Not all configurations were loaded properly."; return -3;}

    std::string content;

    tbb::concurrent_queue<std::string> mq_str;
    tbb::concurrent_queue<std::map<std::string, int>> mq_map;

    std::string poisson_str = "poisson pillow";
    std::map<std::string, int> poisson_map = {{"poisson pillow", 0}};


    int poisson_maps_num = 0;

    ctpl::thread_pool index_threads((int) mc.indexing_threads);
    ctpl::thread_pool merge_threads((int) mc.merging_threads);


    std::vector<std::future<void>> indexing_results(mc.indexing_threads);
    std::vector<std::future<void>> merging_results(mc.merging_threads);

    std::locale loc = boost::locale::generator().generate("en_US.UTF-8");

    for (int i=0; i < mc.indexing_threads; i++){
        indexing_results[i] = index_threads.push(index_thread, std::ref(mq_str), std::ref(mq_map),
                std::ref(poisson_str), std::ref(poisson_map), std::ref(loc));
    }

    for (int i=0; i < mc.merging_threads; i++){
        merging_results[i] = merge_threads.push(merge_thread, std::ref(mq_map), std::ref(poisson_map),
                std::ref(mc.indexing_threads), std::ref(poisson_maps_num));
    }

    auto gen_st_time = get_current_time_fenced();

    MyArchive my_arc;
    std::thread reader(process_deep, std::ref(my_arc), std::ref(mq_str), mc.in_file);

    try
    {
        if (exists(mc.in_file))
        {
            reader.join();
            mq_str.push(poisson_str);
        }
        else {
            std::cerr << mc.in_file << " does not exist\n";
            return -3;
        }
    } catch (const filesystem_error& ex){
        std::cerr << ex.what() << '\n';
        return -2;
    }

    std::cout << "Everything had been READ from archive." << std::endl;

    for (int i = 0; i < mc.indexing_threads; i++){
        indexing_results[i].get();
    }

    auto gen_fn_time = get_current_time_fenced();

    for (int i = 0; i < mc.merging_threads; i++){
        merging_results[i].get();
    }


    std::map<std::string, int> res;

    if (!mq_map.try_pop(res)) {
        std::cerr << "Error in MAP QUEUE" << std::endl;
        return -6;
    }

    std::vector<std::pair<std::string, int> > vector_words;
    for (auto &word: res){
        vector_words.emplace_back(word);
    }

    std::sort(vector_words.begin(), vector_words.end(), [](const auto t1, const auto t2){ return t1.second > t2.second;});
    std::ofstream num_out_f(mc.to_numb_file);
    for (auto &v : vector_words) {
        num_out_f << std::left << std::setw(20) << v.first << ": ";
        num_out_f << std::right << std::setw(10) << std::to_string(v.second) << std::endl;
    }

    std::sort(vector_words.begin(), vector_words.end(), [](const auto t1, const auto t2){ return t1.first.compare(t2.first)<0;});
    std::ofstream alp_out_f(mc.to_alph_file);
    for (auto &v : vector_words) {
        alp_out_f << std::left << std::setw(20) << v.first <<  ": ";
        alp_out_f << std::right << std::setw(10) << std::to_string(v.second) << std::endl;
    }

    std::cout << std::left  << std::setw(35) <<  "General time (read-index): ";
    std::cout << std::right  << std::setw(10) << to_us(gen_fn_time - gen_st_time) << std::endl;

    std::cout << "\nFinished.\n" << std::endl;


    return 0;
}



