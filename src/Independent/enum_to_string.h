//
//  enum_to_string.h
//  Novodb
//
//  Created by mike on 12/29/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#ifndef __Novodb__enum_to_string__
#define __Novodb__enum_to_string__

#include <string>

#include <lldb/API/LLDB.h>

namespace novo {
    std::string state_type_to_string(lldb::StateType state);
    
    std::string address_class_to_string(lldb::AddressClass cl);
    
    std::string section_type_to_string(lldb::SectionType st);
    
    std::string symbol_type_to_string(lldb::SymbolType st);
    
    std::string return_status_to_string(lldb::ReturnStatus status);
}

#endif /* defined(__Novodb__enum_to_string__) */
