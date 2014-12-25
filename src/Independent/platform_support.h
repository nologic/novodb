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

std::vector<std::tuple<int, std::string>> get_process_listing();

int get_page_size();

// on mac this is in the .mm file.
std::string app_path();

// externally pause a process.
int pause_process(int pid);
#endif
