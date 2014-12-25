//
//  platform_support.cpp
//  Novodb
//
//  Created by mike on 9/30/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "../Independent/platform_support.h"

#include <iostream>
#include <errno.h>
#include <stdlib.h>
#include <sys/sysctl.h>
#include <pwd.h>
#include <libproc.h>
#include <unistd.h>

#include <signal.h>

typedef struct kinfo_proc kinfo_proc;

// Taken from: http://stackoverflow.com/questions/18820199/unable-to-detect-application-running-with-another-user-via-switch-user/18821357#18821357
static int GetBSDProcessList(kinfo_proc **procList, size_t *procCount)
// Returns a list of all BSD processes on the system.  This routine
// allocates the list and puts it in *procList and a count of the
// number of entries in *procCount.  You are responsible for freeing
// this list (use "free" from System framework).
// On success, the function returns 0.
// On error, the function returns a BSD errno value.
{
    int                 err;
    kinfo_proc *        result;
    bool                done;
    static const int    name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
    // Declaring name as const requires us to cast it when passing it to
    // sysctl because the prototype doesn't include the const modifier.
    size_t              length;
    
    //    assert( procList != NULL);
    //    assert(*procList == NULL);
    //    assert(procCount != NULL);
    
    *procCount = 0;
    
    // We start by calling sysctl with result == NULL and length == 0.
    // That will succeed, and set length to the appropriate length.
    // We then allocate a buffer of that size and call sysctl again
    // with that buffer.  If that succeeds, we're done.  If that fails
    // with ENOMEM, we have to throw away our buffer and loop.  Note
    // that the loop causes use to call sysctl with NULL again; this
    // is necessary because the ENOMEM failure case sets length to
    // the amount of data returned, not the amount of data that
    // could have been returned.
    
    result = NULL;
    done = false;
    do {
        // Call sysctl with a NULL buffer.
        
        length = 0;
        err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
                     NULL, &length,
                     NULL, 0);
        if (err == -1) {
            err = errno;
        }
        
        // Allocate an appropriately sized buffer based on the results
        // from the previous call.
        
        if (err == 0) {
            result = (kinfo_proc *)malloc(length);
            if (result == NULL) {
                err = ENOMEM;
            }
        }
        
        // Call sysctl again with the new buffer.  If we get an ENOMEM
        // error, toss away our buffer and start again.
        
        if (err == 0) {
            err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
                         result, &length,
                         NULL, 0);
            if (err == -1) {
                err = errno;
            }
            if (err == 0) {
                done = true;
            } else if (err == ENOMEM) {
                free(result);
                result = NULL;
                err = 0;
            }
        }
    } while (err == 0 && ! done);
    
    // Clean up and establish post conditions.
    
    if (err != 0 && result != NULL) {
        free(result);
        result = NULL;
    }
    *procList = result;
    if (err == 0) {
        *procCount = length / sizeof(kinfo_proc);
    }
    
    return err;
}

std::vector<std::tuple<int, std::string>> get_process_listing() {
    using namespace std;
    
    vector<tuple<int, string>> proc_vec;
    kinfo_proc *procList;
    size_t procCount = 0;
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
    
    if(GetBSDProcessList(&procList, &procCount) == 0) {
        for(size_t i = 0; i < procCount; i++) {
            pid_t pid = procList[i].kp_proc.p_pid;
            int allow_dbg = procList[i].kp_proc.p_debugger;
            
            int ret = proc_pidpath (pid, pathbuf, sizeof(pathbuf));
            if ( ret <= 0 ) {
                proc_vec.push_back(make_tuple(pid, string("[no path]")));
            } else {
                proc_vec.push_back(make_tuple(pid, string(pathbuf)));
            }
        }
    }
    
    return proc_vec;
}

int get_page_size() {
    return getpagesize();
}

int pause_process(int pid) {
    return kill(pid, SIGSTOP);
}