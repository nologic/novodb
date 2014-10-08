var db = function() {
    function default_err(resp) {
        log(resp.status + " " + resp.statusText);

        console.info(resp);
    }

    function default_success(resp) {
        log(resp.status + " " + resp.statusText);

        console.info(resp);
    }

    return {
        create_session: function (f_success, f_fail) {
            f_success({$http: angular.element('html').injector().get('$http')});
        },

        attach: function (proc_pid, session, f_success, f_fail) {
            session.$http.get('dbg-llvm://create/target/attach', {
                method: "GET",
                params: {
                    pid: proc_pid
                }
            }).then(function (resp) {
                session.id = resp.data.session;

                if (f_success == undefined) {
                    f_success = default_success;
                }

                f_success(session);
            }, function (resp) {
                if (f_fail == undefined) {
                    f_fail = default_err;
                }

                f_fail(resp);
            });
        },

        load: function (path, args, session, f_success, f_fail) {
            session.$http.get('dbg-llvm://create/target/exe', {
                method: "GET",
                params: {
                    path: path
                }
            }).then(function (resp) {
                session.id = resp.data.session;

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
        },

        launch: function (session, f_success, f_fail) {
            session.$http.get("dbg-llvm://launch", {
                method: "GET",
                params: {
                    session: session.id
                }
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
    };
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