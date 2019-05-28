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

#include "time_measure.h"
#include "config.h"
#include "additional.h"
#include "my_archive.h"

#include <tbb/concurrent_queue.h>

using namespace boost::locale::boundary;


std::map<std::string, int> index_string(std::string content){
    std::locale loc = boost::locale::generator().generate("en_US.UTF-8");

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

void index_thread(tbb::concurrent_queue<std::string*> &mq_str, tbb::concurrent_queue<std::map<std::string, int>*> &mq_map){
    std::string* process_str= nullptr;

    while(true){
        if (mq_str.try_pop(process_str)){
            std::cout << "INDEX🐠 pop 🍏" << std::endl;
            if (process_str == nullptr){
                std::cout << "INDEX🐠 got nullptr ❌" << std::endl;
                break;
            }
            std::map<std::string, int> processed_map = index_string(*process_str);
            std::cout << "INDEX🐠 push 💜" << std::endl;
            mq_map.push(&processed_map);
        }
    }
    mq_map.push(nullptr);
}


std::map<std::string, int> merge_maps(std::vector< std::map<std::string, int> > maps){
    std::map<std::string, int> new_map;
    for (auto &map: maps){
        for (auto &el: map){
            if (new_map.find(el.first) == new_map.end()){
                new_map[el.first] = map[el.first];
            } else{
                new_map[el.first] += map[el.first];
            }
        }
    }
    return new_map;
}



void merge_thread(tbb::concurrent_queue<std::map<std::string, int>*> &mq_maps){
    std::map<std::string, int>* process_map;
    std::vector<std::map<std::string, int>> maps_in_hand;

    while(true){
        if(mq_maps.try_pop(process_map)){
            std::cout << "MERGE🦄 pop 🍏" << std::endl;
            if (process_map == nullptr){
                for (auto &m: maps_in_hand){
                    std::cout << "MERGE🦄 push 💜 before nullptr" << std::endl;
                    mq_maps.push(&m);
                }
                std::cout << "MERGE🦄 got nullptr ❌" << std::endl;
                break;
            } else {
                std::map<std::string, int> pop_map = *process_map;
                maps_in_hand.emplace_back(pop_map);

                if (maps_in_hand.size() >= 2) {
                    std::map<std::string, int> processed_map = merge_maps(maps_in_hand);
                    std::cout << "MERGE🦄 push 💜" << std::endl;
                    mq_maps.push(&processed_map);
                }
            }
        }
    }

//    std::map<std::string, int>* empty_map = nullptr;
    mq_maps.push(nullptr);
}

int main()
{
    std::string conf_file_name = "config.dat";

    // config
    MyConfig mc;
    mc.load_configs_from_file(conf_file_name);
    if (mc.is_configured()) {
        std::cout << "YES! Configurations loaded successfully.\n" << std::endl;
    } else { std::cerr << "Error. Not all configurations were loaded properly."; return -3;}

    std::string content;

    tbb::concurrent_queue<std::string*> mq_str;
    tbb::concurrent_queue<std::map<std::string, int>*> mq_map;


    auto gen_st_time = get_current_time_fenced();

    std::vector<std::thread> all_my_threads;
    unsigned long num_all_threads = mc.indexing_threads + mc.merging_threads;
    all_my_threads.reserve(num_all_threads);

    for (int i=0; i < mc.indexing_threads; i++){
        all_my_threads.emplace_back(index_thread, std::ref(mq_str), std::ref(mq_map));
    }

    for (int i=0; i < mc.merging_threads; i++){
        all_my_threads.emplace_back(merge_thread, std::ref(mq_map));
    }


    // opening archive
    if (is_file_ext(mc.in_file, ".zip")){
        MyArchive arc;
        if (arc.init(mc.in_file) != 0){
            return -4;
        }
        while (arc.next_content_available()){
            content = arc.get_next_content();
            mq_str.push(&content);
        }

    } else{
        std::ifstream in_f(mc.in_file);
        if (! in_f.is_open() || in_f.rdstate())
        { std::cerr << "Couldn't open input-file."; return -2; }
        std::stringstream ss;
        ss << in_f.rdbuf();
        content = ss.str();
        mq_str.push(&content);
    }
    mq_str.push(nullptr);

    for (auto &thr: all_my_threads)
    { thr.join(); }

    auto read_fn_time = get_current_time_fenced();

    std::cout << "Everything had been READ from archive." << std::endl;


    auto index_fn_time = get_current_time_fenced();

    std::map<std::string, int>* res;


    if (!mq_map.try_pop(res)) {
        std::cerr << "Error in MAP QUEUE" << std::endl;
        return -6;
    }

    std::vector<std::pair<std::string, int> > vector_words;
    for (auto word: *res){
        vector_words.emplace_back(word);
    }

    std::sort(vector_words.begin(), vector_words.end(), [](const auto t1, const auto t2){ return t1.second < t2.second;});
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

    auto gen_fn_time = get_current_time_fenced();         //~~~~~~~~~ general finish


    std::cout << std::left  << std::setw(35) <<  "General time (read-index-write): ";
    std::cout << std::right  << std::setw(10) << to_us(gen_fn_time - gen_st_time) << std::endl;
    std::cout << std::left  << std::setw(35) << "Reading time: ";
    std::cout << std::right << std::setw(10) << to_us(read_fn_time - gen_st_time)  << std::endl;
    std::cout << std::left << std::setw(35) << "Indexing time (boost included): " ;
    std::cout << std::right  << std::setw(10) << to_us(index_fn_time - read_fn_time)  << std::endl;

    std::cout << "\nFinished.\n" << std::endl;


    return 0;
}



