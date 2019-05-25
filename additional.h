
#ifndef INTEGRAL_CONCURRENTLY_ADDITIONAL_H
#define INTEGRAL_CONCURRENTLY_ADDITIONAL_H

#include <string>

int is_file_ext(const std::string &file_name, const std::string &ext);

std::string get_file_ext(const std::string &file_name);

int is_float(const std::string &s);

bool is_directory(const std::string& object);

#endif //INTEGRAL_CONCURRENTLY_ADDITIONAL_H
