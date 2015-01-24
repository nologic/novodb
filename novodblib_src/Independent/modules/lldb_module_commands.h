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
        LldbProcessSession(const lldb::SBTarget& _target) : target(_target), is_running(false) {}
        LldbProcessSession(const lldb::SBProcess& _process) : process(_process), target(_process.GetTarget()) {}
    
        lldb::SBProcess process;
        lldb::SBTarget target;
        
        // hack for non-async debugger. async doesn't work correctly.
        bool is_running;
    };
    
    class LldbSessionMap : public std::map<std::string, LldbProcessSession> {
    public:
        LldbSessionMap() : next_id(0) {}
        
        std::string add_session(LldbProcessSession& new_session) {
            std::string new_id = std::to_string(next_id++);
            
            this->insert(std::make_pair(new_id, new_session));
            
            return new_id;
        }
        
        LldbProcessSession& get_session(std::string& session_id) {
            if(this->count(session_id) == 0) {
                // feels a bit out of place but need some similar exception here.
                throw RequestConstraint::Valid::aint("session_getter: session id has no session");
            }
            
            return this->at(session_id);
        }
    private:
        int next_id;
    };
    
    typedef std::function<LldbProcessSession& (std::string& session_id)> session_getter;
 
    void register_commands(RequestRouter& req_router, LldbSessionMap& sessions);
    void register_memops(RequestRouter& req_router, LldbSessionMap& sessions);
}

#endif /* defined(__Novodb__lldb_module_commands1__) */
