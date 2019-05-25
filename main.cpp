#include <thread>

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <list>
#include <functional>
#include <map>

#include "time_measure.h"
#include "config.h"
#include "additional.h"

int main(int argc, char *argv[])
{
    std::string conf_file_name, to_file_alph, to_file_occur;

    if(argc > 1){
        conf_file_name = argv[1];
    } else { conf_file_name = "config.dat"; }

    if (! is_file_ext(conf_file_name, ".txt") && ! is_file_ext(conf_file_name, ".dat") ) {
        std::cerr << "Wrong config file extension." << std::endl;
        return -1;
    }

    // config
    MyConfig mc;
    mc.load_configs_from_file(conf_file_name);
    if (mc.is_configured()) {
        std::cout << "YES! Configurations loaded successfully.\n" << std::endl;
    } else { std::cerr << "Error. Not all configurations were loaded properly."; return -3;}


//    auto gen_st_time = get_current_time_fenced();         //~~~~~~~~~ read start


    std::ifstream in_f(mc.in_file);
    if (! in_f.is_open() || in_f.rdstate()){
        std::cerr << "Couldn't open input-file.";
        return -2;
    }
    std::string content; std::stringstream ss;
    ss << in_f.rdbuf();
    content = ss.str();


    std::cout << "\nOK\n" << std::endl;
    return 0;
}