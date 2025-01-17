//===-- HostThread.h --------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef lldb_Host_HostThread_h_
#define lldb_Host_HostThread_h_

#include "lldb/Core/Error.h"
#include "lldb/lldb-types.h"

#include <memory>

namespace lldb_private
{

class HostNativeThreadBase;

//----------------------------------------------------------------------
/// @class HostInfo HostInfo.h "lldb/Host/HostThread.h"
/// @brief A class that represents a thread running inside of a process on the
///        local machine.
///
/// HostThread allows querying and manipulation of threads running on the host
/// machine.
///
//----------------------------------------------------------------------
class HostThread
{
  public:
    HostThread();
    HostThread(lldb::thread_t thread);

    Error Join(lldb::thread_result_t *result);
    Error Cancel();
    void Reset();
    lldb::thread_t Release();

    void SetState(ThreadState state);
    ThreadState GetState() const;
    HostNativeThreadBase &GetNativeThread();
    const HostNativeThreadBase &GetNativeThread() const;
    lldb::thread_result_t GetResult() const;

    bool EqualsThread(lldb::thread_t thread) const;

  private:
    std::shared_ptr<HostNativeThreadBase> m_native_thread;
};
}

#endif
