#include "additional.h"



// ~~~~~~~~~~~~~~ Return string - extension of the filename. ~~~~~~~~~~~~~~
std::string get_file_ext(const std::string &file_name){
    std::size_t dot_ind = file_name.find_last_of('.');
    return file_name.substr(dot_ind);
}


// ~~~~~~~~~~~~~~ Check if file extension is same as given one. ~~~~~~~~~~~~~~
int is_file_ext(const std::string &file_name, const std::string &ext){
    return !file_name.compare(file_name.length()-ext.length(), ext.length(), ext);
}


// ~~~~~~~~~~~~~~ Check if string can be converted to float. ~~~~~~~~~~~~~~
int is_float(const std::string &s){
    return s.find_first_of(' ') == s.find_last_of(' ');
}


bool is_directory(const std::string& object){
    return object.back() == '/';
}