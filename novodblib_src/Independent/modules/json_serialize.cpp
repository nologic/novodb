//
//  json_serialize.cpp
//  Novodb
//
//  Created by mike on 9/28/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "json_serialize.h"
#include "enum_to_string.h"

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
    using namespace lldb;
    
    pt.put("name", string(symbol.GetName()));
    pt.put("type", novo::symbol_type_to_string(symbol.GetType()));
    
    if(symbol.GetMangledName() != NULL) {
        pt.put("mangled", string(symbol.GetMangledName()));
    }
    
    SBAddress start_addr = symbol.GetStartAddress();
    SBAddress end_addr = symbol.GetEndAddress();
    
    pt.put("file_addr", addr_to_string(start_addr.GetFileAddress()));
    
    if(target != nullptr) {
        addr_t l_start_addr = start_addr.GetLoadAddress(*target);
        
        pt.put("load_addr", addr_to_string(l_start_addr));
        pt.put("size", std::to_string(end_addr.GetLoadAddress(*target) - l_start_addr));
    }
    
    return true;
}

bool to_json(lldb::SBBreakpoint& bp, boost::property_tree::ptree& pt) {
    using namespace lldb;
    using namespace boost::property_tree;
    
    pt.put("id", bp.GetID());
    pt.put("enabled", bp.IsEnabled());
    pt.put("hits", bp.GetHitCount());
    
    SBStream desc;
    if(bp.GetDescription(desc)) {
        pt.put("description", desc.GetData());
    }
    
    size_t locs = bp.GetNumLocations();
    ptree locs_pt;
    
    for(uint32_t l = 0; l < locs; l++) {
        SBBreakpointLocation loc = bp.GetLocationAtIndex(l);
        ptree loc_pt;
        
        loc_pt.put("load_address", addr_to_string(loc.GetLoadAddress()));
        
        locs_pt.push_back(make_pair("", loc_pt));
    }
    
    pt.put_child("locations", locs_pt);
    
    return true;
}