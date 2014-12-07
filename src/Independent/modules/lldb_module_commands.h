//
//  lldb_module_commands1.h
//  Novodb
//
//  Created by mike on 12/6/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#ifndef __Novodb__lldb_module_commands1__
#define __Novodb__lldb_module_commands1__

#define __STDC_LIMIT_MACROS

#include "json_serialize.h"
#include "platform_support.h"
#include "Module.h"

#include <lldb/lldb-private.h>
#include <lldb/Target/Process.h>
#include <lldb/API/LLDB.h>

#include <thread>
#include <array>
#include <iomanip>

#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>

namespace novo {
    class LldbProcessSession {
    public:
        LldbProcessSession(const lldb::SBTarget& _target) : target(_target) {}
        LldbProcessSession(const lldb::SBProcess& _process) : process(_process), target(_process.GetTarget()) {}
    
        lldb::SBProcess process;
        lldb::SBTarget target;
    
    private:
        IMPLEMENT_REFCOUNTING(LldbProcessSession)
    };
 
    void register_commands(RequestRouter& req_router, std::vector<LldbProcessSession>& sessions);
}

#endif /* defined(__Novodb__lldb_module_commands1__) */
