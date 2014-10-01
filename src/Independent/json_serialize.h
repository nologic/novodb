//
//  json_serialize.h
//  Novodb
//
//  Created by mike on 9/28/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#ifndef __Novodb__json_serialize__
#define __Novodb__json_serialize__

#include "lldb/API/LLDB.h"

#include <boost/property_tree/ptree.hpp>

bool to_json(lldb::SBSymbol& symbol, boost::property_tree::ptree& pt, lldb::SBTarget* target);
bool to_json(const lldb::SBAddress& address, boost::property_tree::ptree& pt, lldb::SBTarget* target);
//bool to_json(const lldb::SBModule& symbol, boost::property_tree::ptree& pt);

#endif /* defined(__Novodb__json_serialize__) */
