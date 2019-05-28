#include "config.h"


bool MyConfig::set_in_file(const std::list<std::string> &s_values) {
    std::set<std::string> arch_ext = {".zip", ".targz", ".tar.gz", ".7z"};
    std::string ext = get_file_ext(s_values.front());

    if (ext == ".txt" || arch_ext.find(ext) != arch_ext.end()){
        in_file = s_values.front();
        return true;
    }
    return false;
}


bool MyConfig::set_to_alph_file(const std::list<std::string> &s_values) {
    if ( is_file_ext(s_values.front(), ".txt") ){
        to_alph_file = s_values.front();
        return true;
    }
    return false;
}


bool MyConfig::set_to_numb_file(const std::list<std::string> &s_values) {
    if ( is_file_ext(s_values.front(), ".txt") ){
        to_numb_file = s_values.front();
        return true;
    }
    return false;
}


bool MyConfig::set_num_of_ind_threads(const std::list<std::string> &s_values) {
    int ch = std::stoi(s_values.front());
    if (0 < ch){
        std::istringstream ss (s_values.front());
        return ss >> indexing_threads ? true: false;
    }
    return false;
}


bool MyConfig::set_num_of_mrg_threads(const std::list<std::string> &s_values) {
    int ch = std::stoi(s_values.front());
    if (0 < ch){
        std::istringstream ss (s_values.front());
        return ss >> merging_threads ? true: false;
    }
    return false;
}



bool MyConfig::is_configured(){
    return check_set.empty();
}

int MyConfig::load_configs_from_file(const std::string &f_name){

    std::map< std::string, std::function<bool(const std::list<std::string>&)> > cnf;

    cnf.emplace(std::make_pair("in_file", [this](const std::list<std::string> &s_values)-> bool {return set_in_file(s_values);}));
    cnf.emplace(std::make_pair("to_alph_file", [this](const std::list<std::string> &s_values)-> bool {return set_to_alph_file(s_values);}));
    cnf.emplace(std::make_pair("to_numb_file", [this](const std::list<std::string> &s_values)-> bool {return set_to_numb_file(s_values);}));
    cnf.emplace(std::make_pair("indexing_threads", [this](const std::list<std::string> s_values)-> bool { return set_num_of_ind_threads(s_values);}));
    cnf.emplace(std::make_pair("merging_threads", [this](const std::list<std::string> s_values)-> bool { return set_num_of_mrg_threads(s_values);}));

    try{

        // read config file line by line
        std::ifstream f(f_name);
        if (f){
            std::string line;
            while (getline(f, line)){
                std::list<std::string> content;
                int c = 0;
                for(int i = 0; i < line.length(); i++) {
                    if (isspace(line[i])){
                        if (c!= i){
                            content.emplace_back(line.substr(c, i-c));
                        }
                        c = i+1;
                    }
                }
                if (c != line.length()){ content.emplace_back(line.substr(c, line.length()-c)); }

                // load values into attributes
                std::string cnf_name = content.front();
                content.pop_front();

                if ( cnf.find(cnf_name) != cnf.end()){
                    if ( cnf[cnf_name](content) ){
                        check_set.erase (cnf_name);
                    } else { std::cerr << "Error. Couldn't load" + cnf_name + "\n" << std::endl; return -3; }
                }
            }
            f.close();
            return 0;

        } else { std::cerr << "File coulnd't be opened."; return -1; }
    } catch(std::string &err){ std::cout << err << std::endl; return -2; }
}