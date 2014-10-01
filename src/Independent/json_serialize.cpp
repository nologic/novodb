//
//  json_serialize.cpp
//  Novodb
//
//  Created by mike on 9/28/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "json_serialize.h"

#include <sstream>

std::string addr_to_string(lldb::addr_t address) {
    using namespace std;
    
    stringstream ss;
    ss << hex << address;
    
    return ss.str();
}

bool to_json(lldb::SBSymbol& symbol, boost::property_tree::ptree& pt, lldb::SBTarget* target = nullptr) {
    using namespace std;
    using namespace boost::property_tree;
    
    pt.put("name", string(symbol.GetName()));
    
    if(symbol.GetMangledName() != NULL) {
        pt.put("mangled", string(symbol.GetMangledName()));
    }
    
    ptree start_addr, end_addr;
    to_json(symbol.GetStartAddress(), start_addr, target);
    to_json(symbol.GetEndAddress(), end_addr, target);
    
    pt.put_child("start_addr", start_addr);
    pt.put_child("end_addr", end_addr);

    return true;
}

bool to_json(const lldb::SBAddress& address, boost::property_tree::ptree& pt, lldb::SBTarget* target = nullptr) {
    pt.put("file_addr", addr_to_string(address.GetFileAddress()));
    
    if(target != nullptr) {
        pt.put("load_addr", addr_to_string(address.GetLoadAddress(*target)));
    }
    
    return true;
}