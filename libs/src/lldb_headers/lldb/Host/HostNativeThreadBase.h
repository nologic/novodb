//===-- HostNativeThreadBase.h ----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef lldb_Host_HostNativeThreadBase_h_
#define lldb_Host_HostNativeThreadBase_h_

#include "lldb/Core/Error.h"
#include "lldb/lldb-defines.h"
#include "lldb/lldb-types.h"
#include "lldb/lldb-private-enumerations.h"

namespace lldb_private
{

#if defined(_WIN32)
#define THREAD_ROUTINE __stdcall
#else
#define THREAD_ROUTINE
#endif

class HostNativeThreadBase
{
    friend class ThreadLauncher;
    DISALLOW_COPY_AND_ASSIGN(HostNativeThreadBase);

  public:
    HostNativeThreadBase();
    explicit HostNativeThreadBase(lldb::thread_t thread);
    virtual ~HostNativeThreadBase() {}

    virtual Error Join(lldb::thread_result_t *result) = 0;
    virtual Error Cancel() = 0;
    virtual void Reset();
    lldb::thread_t Release();

    void SetState(ThreadState state);
    ThreadState GetState() const;
    lldb::thread_t GetSystemHandle() const;
    lldb::thread_result_t GetResult() const;

  protected:
    static lldb::thread_result_t THREAD_ROUTINE ThreadCreateTrampoline(lldb::thread_arg_t arg);

    lldb::thread_t m_thread;
    ThreadState m_state;
    lldb::thread_result_t m_result;
};
}

#endif
