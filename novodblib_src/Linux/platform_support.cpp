//
//  platform_support.cpp
//  Novodb
//
//  Created by mike on 9/30/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "platform_support.h"

#include <iostream>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

#include <signal.h>

#include <regex>


std::vector<std::tuple<int, std::string>> get_process_listing() {
    return std::vector<std::tuple<int, std::string>>();
}

int get_page_size() {
    return getpagesize();
}

int pause_process(unsigned long pid) {
    return kill(pid, SIGSTOP);
}

std::vector<vmmap_entry> load_mmap(unsigned long pid) {
    return std::vector<vmmap_entry>();
}
