load_plugin(function() {
    var _instance = undefined;
    var session = undefined;
    var pluginName = "Lldbcmd";

    function Plugin() {}

    Plugin.prototype.attach_ui = function(angular_module, div_container_jq) {
        angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
            return {
                templateUrl: 'plugins/' + pluginName + '/lldb_cmd.html',
                link: function(scope, element, attrs) {
                    scope.base_container = $(element);
                }
            };
        }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile',
            function ($scope, $http, $compile) {
                _instance.set_session($scope.$parent.session);

                $('#lldb_term').terminal(function (command, term) {
                    // execute command!
                    if(command != "") {
                        session.lldbCmd(command, function(data) {
                            if(data.result != "") {
                                data.result.split("\n").forEach(function(line) {
                                    term.echo(line);
                                });
                            } else {
                                log(data);
                            }
                        });
                    }
                }, {
                    greetings: "[[i;red;]This is a pass through terminal to LLDB"
                });

                register_command({
                    cmd: "lldb",
                    complete: function(params) {
                        return ["[lldb command pass through]"];
                    },

                    execute: function(params) {
                        session.lldbCmd(params.join(" "), function(data) {
                            if(data.result != "") {
                                data.result.split("\n").forEach(function(line) {
                                    log(line);
                                });
                            } else {
                                log(data);
                            }
                        }, log);
                    }
                });
            }
        ]);
    }

    Plugin.prototype.set_session = function(new_session) {
        session = new_session;
    };

    Plugin.prototype.set_sessions = function(all_sessions) {

    };

    Plugin.prototype.get_plugin_name = function() {
        return pluginName;
    };

    _instance = new Plugin();

    return _instance;
}());