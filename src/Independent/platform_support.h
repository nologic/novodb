//
//  Header.h
//  Novodb
//
//  Created by mike on 9/30/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#ifndef Novodb_Header_h
#define Novodb_Header_h

#include <vector>
#include <string>

class vmmap_entry {
public:
    vmmap_entry(const std::string& _type, const std::string& _start, const std::string& _end, const std::string& _cur_perm,
                const std::string& _max_perm, const std::string& _sharing_mode, const std::string& _descriptor):
    type(_type), start(_start), end(_end), cur_perm(_cur_perm), max_perm(_max_perm),
    sharing_mode(_sharing_mode), descriptor(_descriptor) {
    }
    
    const std::string type;
    const std::string start;
    const std::string end;
    const std::string cur_perm;
    const std::string max_perm;
    const std::string sharing_mode;
    const std::string descriptor;
};

std::vector<std::tuple<int, std::string>> get_process_listing();

int get_page_size();

// on mac this is in the .mm file.
std::string app_path();

// externally pause a process.
int pause_process(unsigned long pid);

std::vector<vmmap_entry> load_mmap(unsigned long pid);
#endif
