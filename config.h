#ifndef COUNTING_WORDS_CONFIG_H
#define COUNTING_WORDS_CONFIG_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <functional>
#include <unordered_set>
#include <vector>
#include <list>
#include <set>
#include "boost/filesystem.hpp"


class MyConfig{

public:
    std::string in_file;
    std::string to_alph_file;
    std::string to_numb_file;
    unsigned int indexing_threads=0;
    unsigned int merging_threads=0;

private:
    std::unordered_set<std::string> check_set = {"in_file", "to_alph_file", "to_numb_file",
                                                 "indexing_threads", "merging_threads"};

public:
    bool set_in_file(const std::list<std::string> &s_values);
    bool set_to_alph_file(const std::list<std::string> &s_values);
    bool set_to_numb_file(const std::list<std::string> &s_values);
    bool set_num_of_ind_threads(const std::list<std::string> &s_values);
    bool set_num_of_mrg_threads(const std::list<std::string> &s_values);

    bool is_configured();
    int load_configs_from_file(const std::string &f_name);
};


#endif //COUNTING_WORDS_CONFIG_H