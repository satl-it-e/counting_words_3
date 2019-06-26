#include "my_archive.h"

//#define print_wrong_file_extensions

namespace fs = boost::filesystem;

std::string MyArchive::read_content() {
    size_t total = archive_entry_size(entry);
    char *buf = (char *) malloc(total);
    ssize_t size = archive_read_data(a, buf, total);
    if (size <= 0) {
        // Error handling
    }
    std::string str(buf);
    return str;
}


int MyArchive::init(std::string& filename){
    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    archive_read_support_format_raw(a);
    int r = archive_read_open_filename(a, filename.c_str(), 10240); // Note 1
    if (r != ARCHIVE_OK) {
        std::cerr << "Couldn't open archive " << filename << std::endl;
        return -1;
    }
    is_arch = true;
    return 0;
}

bool MyArchive::next_content_available(){
    if (is_arch){
        if (archive_read_next_header(a, &entry) != ARCHIVE_OK){
            is_arch = false;
            int r = archive_read_free(a);
            if (r != ARCHIVE_OK){
                std::cerr << "Something wrong with cleaning archive" << std::endl;
            }
            return false;
        }
        return true;
    }
    return false;
}

std::string MyArchive::get_next_content(){

    const char* filename = archive_entry_pathname(entry);
    std::string filename_str(filename);
    if (fs::is_directory(filename)){
#ifdef print_wrong_file_extensions
        std::cerr << "Directory in archive" << std::endl;
#endif
        archive_read_data_skip(a);
    } else {
        std::transform(filename_str.begin(), filename_str.end(), filename_str.begin(), ::tolower);
        if (boost::filesystem::extension(filename_str) == ".txt") {
            return read_content();
        }
#ifdef print_wrong_file_extensions
        else{
                    std::cerr << "Wrong file extension of file (not .txt) " << filename << std::endl;
                }
#endif
    }
    return "";
}
