function create_ndb_session($http) {
    var session_id = null;
    var mem_separator = ' ';

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
            return func(resp.data);
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

    NdbSession.prototype.attach = function(proc_pid, f_success, f_fail) {
        url_get_passthrough("dbg-llvm://create/target/attach", {
            pid: proc_pid
        }, extract_data(function(sdata) {
            session_id = sdata.session;

            if (f_success == undefined) {
                f_success = default_success;
            }

            f_success(sdata);
        }), f_fail);
    };

    NdbSession.prototype.load = function (path, args, f_success, f_fail) {
        url_get_passthrough("dbg-llvm://create/target/exe", {
            path: path
        }, extract_data(function(resp) {
            session_id = sdata.session;

            if (f_success == undefined) {
                f_success = default_success;
            }

            f_success(sdata);
        }), f_fail);
    };

    NdbSession.prototype.launch = function (f_success, f_fail) {
        url_get_passthrough("dbg-llvm://launch", {
            session: session_id
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.getThreads = function (f_success, f_fail) {
        url_get_passthrough("dbg-llvm://list/threads", {
            session: session_id
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.getModules = function (f_success, f_fail) {
        url_get_passthrough("dbg-llvm://list/modules", {
            session: session_id
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.getFrames = function (thread_ind, f_success, f_fail) {
        url_get_passthrough("dbg-llvm://list/frames", {
            session: session_id,
            thread: thread_ind
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.getSymbols = function (module, f_success, f_fail) {
        url_get_passthrough("dbg-llvm://list/symbols", {
            session: session_id,
            module: module
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.getProcState = function (f_success, f_fail) {
        url_get_passthrough("dbg-llvm://proc/state", {
            session: session_id
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.setBreakpoint = function(symbol, f_success, f_fail) {
        url_get_passthrough("dbg-llvm://breakpoint/set", {
            session: session_id,
            symbol: symbol
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.readInstructions = function(address, count, f_success, f_fail) {
        url_get_passthrough("dbg-llvm://read/instructions", {
            session: session_id,
            address: address,
            count: count
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.readMemory = function(address, count, f_success, f_fail) {
        url_get_passthrough("dbg-llvm://read/memory", {
            session: session_id,
            address: address,
            count: count,
            sep: mem_separator
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.writeMemory = function(address, count, f_success, f_fail) {
        f_fail("Not yet implemented");
    };

    NdbSession.prototype.readRegisters = function(frame, thread, f_success, f_fail) {
        url_get_passthrough("dbg-llvm://list/registers", {
            session: session_id,
            frame: frame,
            thread: thread
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.writeRegisters = function(frame, thread, f_success, f_fail) {
        f_fail("Not yet implemented");
    };

    // EIP commands
    NdbSession.prototype.step = function(thread, f_success, f_fail) {
        url_get_passthrough("dbg-llvm://cmd/step", {
            session: session_id,
            thread: thread
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.resume = function(thread, f_success, f_fail) {
        url_get_passthrough("dbg-llvm://cmd/resume", {
            session: session_id,
            thread: thread
        }, extract_data(f_success), f_fail);
    };

    return new NdbSession();
}

function log(txt) {
    console.info(txt);

    if(typeof txt === 'object') {
        txt = JSON.stringify(txt, null, '\t');
    }

    var log_pre = $('#log_view');
    log_pre.append(txt);
    log_pre.append('\n');
}