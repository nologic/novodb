// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_app.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>

#include "simple_handler.h"
#include "util.h"
#include "dbg_handler.h"
#include "request_router.h"
#include "llvm_scheme_handler.h"
#include "util_scheme_handler.h"
#include "platform_support.h"

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_url.h"

std::string slurp(const std::string& path) {
    std::ifstream in(path, std::ios::in | std::ios::binary);
    
    std::stringstream sstr;
    sstr << in.rdbuf();
    
    in.close();
    
    return sstr.str();
}

void for_each_plugin(boost::filesystem::path frontend_path, std::function<void (std::string&, boost::property_tree::ptree&)> pl_func) {
    using namespace std;
    using namespace boost::filesystem;
    
    path plugins_path(frontend_path / path("plugins"));
    
    if(!exists(plugins_path)) {
        throw string("plugins path does not exist.");
    }
    
    if(!is_directory(plugins_path)) {
        throw string("plugins path is not a directory.");
    }
    
    for(directory_iterator dir_itr(plugins_path); dir_itr != directory_iterator(); dir_itr++) {
        path def_file = *dir_itr / path("definition.json");
        
        if(exists(def_file)) {
            boost::property_tree::ptree plugin_def;
            string plugin_name = dir_itr->path().filename().string();
            
            boost::property_tree::json_parser::read_json(def_file.string(), plugin_def);
            
            pl_func(plugin_name, plugin_def);
        }
    }
}

std::string ptree_tostring(boost::property_tree::ptree& tree) {
    std::stringstream ss;
    boost::property_tree::write_json(ss, tree, false);
    
    return ss.str();
}

std::string pre_process_url(const std::string& url) {
    using namespace std;
    using namespace boost::filesystem;
    
    if(!boost::starts_with(url, "file://")) {
        throw std::string("Can't preprocess files not on file://");
    }
    
    std::string base_path = url.substr(7);
    std::string data = slurp(base_path + "/index.html");
    
    stringstream js_files;
    stringstream css_files;
    
    for_each_plugin(path(base_path), [&js_files, &css_files] (std::string& name, boost::property_tree::ptree& plugin_def) {
        boost::property_tree::ptree& jstree = plugin_def.get_child("js_files");
        boost::property_tree::ptree& csstree = plugin_def.get_child("css_files");
        
        for(auto js : jstree) {
            string file = js.second.data();

            js_files << "<script type=\"text/javascript\" src=\"plugins/" << name << "/" << file << "\"></script>" << endl;
        }
        
        for(auto css : csstree) {
            string file = css.second.data();
            
            css_files << "<link rel=\"stylesheet\" href=\"plugins/" << name << "/" << file << "\">" << endl;
        }
    });
    
    boost::replace_all(data, "{{js_files}}", js_files.str());
    boost::replace_all(data, "{{css_files}}", css_files.str());
    
    ofstream out(base_path + "/index-patched.html", std::ios::out | std::ios::binary);
    out << data;
    out.close();
    
    return url + "/index-patched.html";
}

void SimpleApp::OnContextInitialized() {
  REQUIRE_UI_THREAD();

  // Information used when creating the native window.
  CefWindowInfo window_info;

#if defined(OS_WIN)
  // On Windows we need to specify certain flags that will be passed to
  // CreateWindowEx().
  window_info.SetAsPopup(NULL, "Novodb");
#endif

    // SimpleHandler implements browser-level callbacks.
    CefRefPtr<SimpleHandler> handler(new SimpleHandler());

    // Specify CEF browser settings here.
    CefBrowserSettings browser_settings;

    // this might not last but we can work around if this option
    // is taken away.
    browser_settings.web_security = STATE_DISABLED;

    std::string url("file://" + app_path());
    bool append_file = true;

    // Check if a "--url=" value was provided via the command-line. If so, use
    // that instead of the default URL.
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();

    if (command_line->HasSwitch("url")) {
        url = command_line->GetSwitchValue("url");
        append_file = false;
    }
    
    if(command_line->HasSwitch("novo-ui-base")) {
        url = command_line->GetSwitchValue("novo-ui-base");
        
        if(!command_line->HasSwitch("no-preprocess")) {
            url = pre_process_url(url);
            append_file = false;
        }
    }
    
    if(append_file) {
        url += "/frontend/index.html";
    }

    novo::install_llvm_scheme();
    novo::install_util_scheme();
    
    // Create the first browser window.
    CefBrowserHost::CreateBrowser(window_info, handler.get(), url, browser_settings, NULL);
}
