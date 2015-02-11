//
//  platform_support.cpp
//  Novodb
//
//  Created by mike on 9/30/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "platform_support.h"

#include <iostream>
#include <fstream>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#include <signal.h>

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

std::vector<std::tuple<int, std::string>> get_process_listing() {
    using namespace std;
    using namespace boost::filesystem;
    
    path proc("/proc");

    if(!exists(proc)) {
        throw string("/proc does not exist.");
    }
    
    if(!is_directory(proc)) {
        throw string("/proc is not a directory.");
    }

    boost::regex expression("[[:digit:]]+");
    std::vector<std::tuple<int, std::string>> output;

    for(directory_iterator dir_itr(proc); dir_itr != directory_iterator(); dir_itr++) {
        string dirname = dir_itr->path().filename().string();

        boost::smatch results;
	if(dirname.size() < 11 && boost::regex_match(dirname, expression)) {
            // This is a numerical dir, a pid;
            path exe = dir_itr->path() / "exe";
            string description;

            try {
                description = canonical(exe).string();
            } catch(filesystem_error& err) {
            }

            if(description.empty()) {
               ifstream ifs( (dir_itr->path() / "stat").string(), ios::in);
               getline(ifs, description, ' ');
               getline(ifs, description, ' ');
               ifs.close();
            }

            output.push_back(make_tuple(stoi(dirname), description));
        }
    }

    return output;
}

int get_page_size() {
    return getpagesize();
}

int pause_process(unsigned long pid) {
    return kill(pid, SIGSTOP);
}

std::vector<vmmap_entry> load_mmap(unsigned long pid) {
    using namespace std;
    using namespace boost::filesystem;

    vector<vmmap_entry> output;

    // ([0-9a-f]*)-([0-9a-f]*) ([prwx-]{4}) ([0-9]*) ([0-9]{2}:[0-9]{2}) ([0-9]*) *(.*)
    path file = "/proc/" + std::to_string(pid) + "/maps";

    if(exists(file)) {
        boost::regex expression("([0-9a-f]*)-([0-9a-f]*) ([prwx-]{4}) ([0-9a-f]*) ([0-9a-f]{2}:[0-9a-f]{2}) ([0-9a-f]*) *(.*)");

        ifstream ifs(file.string(), ios::in);
        string line;

        while(getline(ifs, line)) {
            boost::smatch results;

            if(regex_search(line, results, expression)) {
                vmmap_entry entry("", results[1].str(), results[2].str(), results[3].str(),
                                  "", "", results[6].str());
            
                output.push_back(entry);
            } else {
                cerr << "mismatch on: " << line << endl;
            }
        }

        ifs.close();
    }

    return output;
}
