function create_ndb_session($http) {
    var session_id = null;
    var mem_separator = ' ';

    // observable cached variables.
    var stepCount = 0;
    var procState = { state: 0, description: "invalid" };
    var selectedThread = undefined;
    var currentPc = undefined;

    function NdbSession() {}

    function default_err(resp) {
        log(resp.status + " " + resp.statusText);

        console.info(resp);
    }

    function default_success(resp) {
        log(resp.status + " " + resp.statusText);

        console.info(resp);
    }

    function extract_data(func) {
        return function(resp){
            if(func == undefined) {
                return;
            } else if(typeof(func) == "function") {
                return func(resp.data);
            } else {
                // we actually have an array of functions to exec.
                func.forEach(function(f) {
                    if(f != undefined) {
                        f(resp.data);
                    }
                });
            }
        }
    }

    function url_get_passthrough(url, params, f_success, f_fail) {
        $http.get(url, {
            method: "GET",
            params: params
        }).then(function (resp) {
            if (f_success == undefined) {
                f_success = default_success;
            }

            f_success(resp);
        }, function (resp) {
            if (f_fail == undefined) {
                f_fail = default_err;
            }

            f_fail(resp);
        });
    }

    // cached values
    NdbSession.prototype.get_selectedThread = function() {
        return selectedThread;
    }

    NdbSession.prototype.get_stepCount = function() {
        return stepCount;
    }

    NdbSession.prototype.get_procState = function() {
        return procState;
    }

    NdbSession.prototype.get_currentPc = function() {
        return currentPc;
    }

    // backend function
    NdbSession.prototype.attach = function(proc_pid, f_success, f_fail) {
        url_get_passthrough("dbg-lldb://create/target/attach", {
            pid: proc_pid
        }, extract_data([function(sdata) {
            session_id = sdata.session;            
        },
        f_success]), f_fail);
    };

    NdbSession.prototype.load = function (path, args, f_success, f_fail) {
        url_get_passthrough("dbg-lldb://create/target/exe", {
            path: path
        }, extract_data([function(sdata) {
            session_id = sdata.session;
        }, f_success]), f_fail);
    };

    NdbSession.prototype.launch = function (f_success, f_fail) {
        url_get_passthrough("dbg-lldb://launch", {
            session: session_id
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.getThreads = function (f_success, f_fail) {
        url_get_passthrough("dbg-lldb://list/threads", {
            session: session_id
        }, extract_data([function(data) {
            selectedThread = data.threads.filter(function(th) {
                return ('current' in th) && th.current == "true";
            })[0];
        }, f_success]), f_fail);
    };

    NdbSession.prototype.getModules = function (f_success, f_fail) {
        url_get_passthrough("dbg-lldb://list/modules", {
            session: session_id
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.getFrames = function (thread_ind, f_success, f_fail) {
        url_get_passthrough("dbg-lldb://list/frames", {
            session: session_id,
            thread: thread_ind
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.getSymbols = function (module, f_success, f_fail) {
        url_get_passthrough("dbg-lldb://list/symbols", {
            session: session_id,
            module: module
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.getProcState = function (f_success, f_fail) {
        url_get_passthrough("dbg-lldb://proc/state", {
            session: session_id
        }, extract_data([function(data) {
            procState = data;
        },
        f_success]), f_fail);
    };

    NdbSession.prototype.setBreakpoint = function(symbol, f_success, f_fail) {
        url_get_passthrough("dbg-lldb://breakpoint/set", {
            session: session_id,
            symbol: symbol
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.readInstructions = function(address, count, f_success, f_fail) {
        url_get_passthrough("dbg-lldb://read/instructions", {
            session: session_id,
            address: address,
            count: count
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.readMemory = function(address, count, f_success, f_fail) {
        url_get_passthrough("dbg-lldb://read/memory", {
            session: session_id,
            address: address,
            count: count,
            sep: mem_separator
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.writeMemory = function(address, count, f_success, f_fail) {
        f_fail("Not yet implemented");
    };

    NdbSession.prototype.writeByte = function(address, byte, f_success, f_fail) {
        url_get_passthrough("dbg-lldb://write/byte", {
            session: session_id,
            address: address,
            'byte': byte
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.readRegisters = function(frame, thread, f_success, f_fail) {
        url_get_passthrough("dbg-lldb://list/registers", {
            session: session_id,
            frame: frame,
            thread: thread
        }, extract_data([function(data) {
            currentPc = data.registers[0].values.filter(function(regval) {
                return regval.name == "rip" || regval.name == "eip";
            })[0];
        }, f_success]), f_fail);
    };

    NdbSession.prototype.writeRegisters = function(frame, thread, f_success, f_fail) {
        f_fail("Not yet implemented");
    };

    // EIP commands
    NdbSession.prototype.step = function(thread, f_success, f_fail) {
        url_get_passthrough("dbg-lldb://cmd/step", {
            session: session_id,
            thread: thread
        }, extract_data([function(data) {
            // returned from step.
            stepCount += 1;
        }, f_success]), f_fail);
    };

    NdbSession.prototype.resume = function(thread, f_success, f_fail) {
        url_get_passthrough("dbg-lldb://cmd/resume", {
            session: session_id,
            thread: thread
        }, extract_data(f_success), f_fail);
    };
    // // end backend functions.

    // Front end functions

    // // end Front end functions

    return new NdbSession();
}

function log(txt) {
    console.info(txt);

    if(typeof txt === 'object') {
        txt = JSON.stringify(txt, null, '\t');
    }

    output(txt);
}

function output(txt) {
    $('#footer_terminal').terminal().echo(txt);
}