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

std::vector<std::tuple<int, bool, std::string>> get_process_listing();

#endif
