var db = function() {
    function default_err(resp) {
        log(resp.status + " " + resp.statusText);

        console.info(resp);
    }

    function default_success(resp) {
        log(resp.status + " " + resp.statusText);

        console.info(resp);
    }

    function url_get_passthrough(url, params, session, f_success, f_fail) {
        if(session.id != undefined) {
            params.session = session.id;
        }

        session.$http.get(url, {
            method: "GET",
            params: params
        }).then(function (resp) {
            if (f_success == undefined) {
                f_success = default_success;
            }

            f_success(session, resp);
        }, function (resp) {
            if (f_fail == undefined) {
                f_fail = default_err;
            }

            f_fail(resp);
        });
    }

    return {
        create_session: function (f_success, f_fail) {
            f_success({$http: angular.element('html').injector().get('$http')});
        },

        attach: function (proc_pid, session, f_success, f_fail) {
            url_get_passthrough("dbg-llvm://create/target/attach", {pid: proc_id}, session, function(resp) {
                session.id = resp.data.session;

                if (f_success == undefined) {
                    f_success = default_success;
                }

                f_success(session, resp);
            }, f_fail);
        },

        load: function (path, args, session, f_success, f_fail) {
            url_get_passthrough("dbg-llvm://create/target/exe", {path: path}, session, function(resp) {
                session.id = resp.data.session;

                if (f_success == undefined) {
                    f_success = default_success;
                }

                f_success(session, resp);
            }, f_fail);
        },

        launch: function (session, f_success, f_fail) {
            url_get_passthrough("dbg-llvm://launch", {}, session, f_success, f_fail);
        },

        getThreads: function (session, f_success, f_fail) {
            url_get_passthrough("dbg-llvm://list/threads", {}, session, f_success, f_fail);
        },

        getModules: function (session, f_success, f_fail) {
            url_get_passthrough("dbg-llvm://list/modules", {}, session, f_success, f_fail);
        },

        getFrames: function (thread_ind, session, f_success, f_fail) {
            url_get_passthrough("dbg-llvm://list/frames", { thread: thread_ind }, session, f_success, f_fail);
        },

        getSymbols: function (module, session, f_success, f_fail) {
            url_get_passthrough("dbg-llvm://list/symbols", { module: module }, session, f_success, f_fail);
        },

        getProcState: function (session, f_success, f_fail) {
            url_get_passthrough("dbg-llvm://proc/state", {}, session, f_success, f_fail);
        },

        setBreakpoint: function(symbol, session, f_success, f_fail) {
            url_get_passthrough("dbg-llvm://breakpoint/set", { symbol: symbol }, session, f_success, f_fail);
        },

        readMemory: function(address, count, separator, session, f_success, f_fail) {
            url_get_passthrough("dbg-llvm://breakpoint/set", {
                address: address,
                count: count,
                sep: separator
            }, session, f_success, f_fail);
        },

        writeMemory: function(address, count, separator, session, f_success, f_fail) {
            f_fail("Not yet implemented");
        },

        readRegisters: function(frame, thread, separator, session, f_success, f_fail) {
            url_get_passthrough("dbg-llvm://list/registers", {
                frame: frame,
                thread: thread
            }, session, f_success, f_fail);
        },

        writeRegisters: function(frame, thread, separator, session, f_success, f_fail) {
            f_fail("Not yet implemented");
        }
    }
}();

function log(txt) {
    console.info(txt);

    if(typeof txt === 'object') {
        txt = JSON.stringify(txt, null, '\t');
    }

    var log_pre = $('#log_view');
    log_pre.append(txt);
    log_pre.append('\n');
}