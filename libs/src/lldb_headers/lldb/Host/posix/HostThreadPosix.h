//===-- HostThreadWindows.h -------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef lldb_Host_posix_HostThreadPosix_h_
#define lldb_Host_posix_HostThreadPosix_h_

#include "lldb/Host/HostNativeThreadBase.h"

namespace lldb_private
{

class HostThreadPosix : public HostNativeThreadBase
{
    DISALLOW_COPY_AND_ASSIGN(HostThreadPosix);

  public:
    HostThreadPosix();
    HostThreadPosix(lldb::thread_t thread);
    virtual ~HostThreadPosix();

    virtual Error Join(lldb::thread_result_t *result);
    virtual Error Cancel();

    Error Detach();
};
}

#endif
