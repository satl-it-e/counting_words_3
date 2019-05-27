#ifndef COUNTING_WORDS_MY_ARCHIVE_H
#define COUNTING_WORDS_MY_ARCHIVE_H

#include <iostream>

#include <sys/types.h>

#include <sys/stat.h>

#include <archive.h>
#include <archive_entry.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "additional.h"



class MyArchive{

    private:
        struct archive *a;
        struct archive_entry *entry;
        bool is_arch = false;
    
        std::string read_content();
    
    public:
        int init(std::string& filename);
    
        bool next_content_available();
    
        std::string get_next_content();

};





#endif //COUNTING_WORDS_MY_ARCHIVE_H
