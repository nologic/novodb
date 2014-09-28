// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_app.h"

#include <string>
#include <iostream>

#include "simple_handler.h"
#include "util.h"
#include "dbg_handler.h"
#include "request_router.h"
#include "llvm_scheme_handler.h"

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_url.h"

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

  std::string url;

  // Check if a "--url=" value was provided via the command-line. If so, use
  // that instead of the default URL.
  CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();
  url = command_line->GetSwitchValue("url");
  if (url.empty()) {
        url = "file://./html/index.html";
  }

  novo::install_llvm_scheme();
    
  // Create the first browser window.
  CefBrowserHost::CreateBrowser(window_info, handler.get(), url, browser_settings, NULL);
}
