function create_ndb_session($http) {
    var mem_separator = ' ';

    // observable cached variables.
    var stepCount = 0;
    var procState = { state: 0, description: "invalid" };
    var selectedThread = undefined;
    var currentPc = undefined;
    var mem_maps = undefined;

    // "constant" values they shouldn't change across session.
    var attachInfo = null;
    var session_id = null;

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
                return resp.data;
            } else if(typeof(func) == "function") {
                return func(resp.data);
            } else {
                // we actually have an array of functions to exec.
                return func.reduce(function(acc, f) {
                    if(f != undefined) {
                        return f(resp.data);
                    }
                }, undefined);
            }
        }
    }

    function url_get_passthrough(path, params, f_success, f_fail) {
        return $http({
            url: window.path_to_url("dbg-lldb", path),
            method: window.call_method,
            params: params
        }).then(function (resp) {
            if (f_success == undefined) {
                f_success = default_success;
            }
            
            if(call_method == "JSONP" && 'data' in resp) {
                // mask it to make it look like we did a regular GET.
                resp.status = resp.data.code;
                resp.statusText = resp.msg;
                resp.data = resp.data.output;
            }
            
            if(call_method == "JSONP" && resp.status != 200) {
                if (f_fail == undefined) {
                    f_fail = default_err;
                }

                return f_fail(resp);
            } else {
                return f_success(resp);
            }

            return f_success(resp);
        }, function (resp) {
            if (f_fail == undefined) {
                f_fail = default_err;
            }

            if(call_method == "JSONP" && 'data' in resp) {
                resp.status = resp.data.code;
                resp.statusText = resp.data.msg;
                resp.data = resp.data.output;
            }

            return f_fail(resp);
        });
    }

    // cached values
    NdbSession.prototype.get_selectedThread = function() {
        return selectedThread;
    };

    NdbSession.prototype.get_stepCount = function() {
        return stepCount;
    };

    NdbSession.prototype.get_procState = function() {
        return procState;
    };

    NdbSession.prototype.get_currentPc = function() {
        return currentPc;
    };

    NdbSession.prototype.get_mem_maps = function() {
        return mem_maps;
    };

    // constant accessors
    NdbSession.prototype.get_attach_info = function() {
        return attachInfo;
    };

    NdbSession.prototype.get_architecture = function() {
        return attachInfo.triple.split("-")[0];
    }

    // backend function
    NdbSession.prototype.listSessions = function(f_success, f_fail) {
        url_get_passthrough("sessions", {
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.attachSession = function(sdata, f_success, f_fail) {
        attachInfo = sdata;

        session_id = sdata.session;
        selectedThread = {
            tid: sdata.current_tid
        };

        setTimeout(function(){
            f_success(sdata);
        }, 0);
    };

    NdbSession.prototype.attach = function(proc_pid, f_success, f_fail) {
        url_get_passthrough("create/target/attach", {
            pid: proc_pid
        }, extract_data([function(sdata) {
            attachInfo = sdata;

            session_id = sdata.session;
            selectedThread = {
                tid: sdata.current_tid
            };
        },
        f_success]), f_fail);
    };

    NdbSession.prototype.connect = function(url, f_success, f_fail) {
        url_get_passthrough("create/target/remote", {
            url: url
        }, extract_data([function(sdata) {
            attachInfo = sdata;

            session_id = sdata.session;
            selectedThread = {
                tid: sdata.current_tid
            };
        },
        f_success]), f_fail);
    };

    NdbSession.prototype.load = function (path, args, f_success, f_fail) {
        url_get_passthrough("create/target/exe", {
            path: path
        }, extract_data([function(sdata) {
            session_id = sdata.session;
        }, f_success]), f_fail);
    };

    NdbSession.prototype.launch = function (f_success, f_fail) {
        url_get_passthrough("launch", {
            session: session_id
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.getThreads = function (f_success, f_fail) {
        url_get_passthrough("list/threads", {
            session: session_id
        }, extract_data([function(data) {
            selectedThread = data.threads.filter(function(th) {
                return ('current' in th) && th.current == "true";
            })[0];
        }, f_success]), f_fail);
    };

    NdbSession.prototype.getModules = function (f_success, f_fail) {
        url_get_passthrough("list/modules", {
            session: session_id
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.getFrames = function (thread_ind, f_success, f_fail) {
        url_get_passthrough("list/frames", {
            session: session_id,
            thread: thread_ind
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.getSymbols = function (module, f_success, f_fail) {
        url_get_passthrough("list/symbols", {
            session: session_id,
            module: module
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.searchSymbols = function (module, start_ind, count, max_matches, regex, f_success, f_fail) {
        url_get_passthrough("search/symbols", {
            session: session_id,
            module: module,
            index: start_ind,
            count: count,
            max_matches: max_matches,
            regex: regex
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.getProcState = function (f_success, f_fail) {
        url_get_passthrough("proc/state", {
            session: session_id
        }, extract_data([function(data) {
            procState = data;
        },
        f_success]), f_fail);
    };

    // type = {"symbol", "address"}
    NdbSession.prototype.addBreakpoint = function(point, type, f_success, f_fail) {
        if(type == "symbol") {
            url_get_passthrough("add/breakpoint", {
                session: session_id,
                symbol: point
            }, extract_data(f_success), f_fail);
        } else if(type == "address") {
            url_get_passthrough("add/breakpoint", {
                session: session_id,
                address: point
            }, extract_data(f_success), f_fail);
        } else {
            f_fail({
                error: "unknown type info"
            });
        }
    };

    NdbSession.prototype.readInstructions = function(address, count, f_success, f_fail) {
        return url_get_passthrough("read/instructions", {
            session: session_id,
            address: address,
            count: count
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.readMemory = function(address, count, f_success, f_fail) {
        return url_get_passthrough("read/memory", {
            session: session_id,
            address: address,
            count: count,
            sep: mem_separator
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.yaraSearch = function(address, length, pattern, max_matches, f_success, f_fail) {
        return url_get_passthrough("search/memory/yara", {
            session: session_id,
            address: address,
            length: length,
            pattern: pattern,
            max_matches: max_matches
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.yaraSearchResults = function(path, f_success, f_fail) {
        return url_get_passthrough(path, {
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.writeMemory = function(address, count, f_success, f_fail) {
        f_fail("Not yet implemented");
    };

    NdbSession.prototype.writeByte = function(address, byte, f_success, f_fail) {
        return url_get_passthrough("write/byte", {
            session: session_id,
            address: address,
            'byte': byte
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.readRegisters = function(frame, thread, f_success, f_fail) {
        if(thread === undefined) {
            thread = selectedThread.tid;
        }

        if(frame === undefined) {
            frame = procState.frame;
        }

        return url_get_passthrough("list/registers", {
            session: session_id,
            frame: frame,
            tid: thread
        }, extract_data([function(data) {
            currentPc = data.registers[0].values.filter(function(regval) {
                // not general, but ok for now.
                return regval.name == "rip" || regval.name == "eip" || regval.name == "pc";
            })[0];
        }, f_success]), f_fail);
    };

    NdbSession.prototype.listRegions = function(f_success, f_fail) {
        return url_get_passthrough("list/regions", {
            session: session_id
        }, extract_data([function(data) {
            mem_maps = data.regions;
        }, f_success]), f_fail);
    };

    NdbSession.prototype.writeRegisters = function(frame, thread, f_success, f_fail) {
        f_fail("Not yet implemented");
    };

    // EIP commands
    NdbSession.prototype.step = function(tid, f_success, f_fail) {
        if(tid == undefined) {
            tid = selectedThread.tid;
        }

        return url_get_passthrough("cmd/step", {
            session: session_id,
            tid: tid
        }, extract_data([function(data) {
            // returned from step.
            stepCount += 1;
        }, f_success]), f_fail);
    };

    NdbSession.prototype.continue_proc = function(f_success, f_fail) {
        return url_get_passthrough("cmd/continue", {
            session: session_id
        }, extract_data([function(data){
            // returned means we've stopped.
            stepCount += 1;
        }, f_success]), f_fail);
    };

    NdbSession.prototype.stop_proc = function(f_success, f_fail) {
        return url_get_passthrough("cmd/stop", {
            session: session_id
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.detach = function(f_success, f_fail) {
        return url_get_passthrough("cmd/detach", {
            session: session_id
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.lldbCmd = function(cmd, f_success, f_fail) {
        return url_get_passthrough("cmd/lldb", {
            session: session_id,
            cmd: cmd
        }, extract_data([function(data){
            // returned means we've stopped.
            stepCount += 1;
        }, f_success]), f_fail);
    };

    NdbSession.prototype.lldbCmdComplete = function(cmd, cursor, maxmatches, f_success, f_fail) {
        return url_get_passthrough("cmd/lldb/complete", {
            session: session_id,
            cmd: cmd,
            cursor: cursor
        }, extract_data(f_success), f_fail);
    };

    NdbSession.prototype.lldbCmdNoStep = function(cmd, f_success, f_fail) {
        return url_get_passthrough("cmd/lldb", {
            session: session_id,
            cmd: cmd
        }, extract_data(f_success), f_fail);
    }; 
    // // end backend functions.

    // Front end functions
    var destroy_listeners = [];
    NdbSession.prototype.add_destroy_listener = function(listener) {
        destroy_listeners.push(listener);
    };

    NdbSession.prototype.destroy = function() {
        destroy_listeners.forEach(function(listener){
            listener();
        });
    }
    // // end Front end functions

    return new NdbSession();
}

function log(txt) {
    if(typeof txt === 'object') {
        txt = JSON.stringify(txt, null, '\t');
    }

    console.info(txt);

    if(window.output) {
        window.output(txt);
    }
}