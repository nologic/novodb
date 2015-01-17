(function() {
    var pluginName = "Lldbcmd";

    var pluginSpec = {
        name: pluginName,
        type: undefined,

        staticInit: function(angular_module) {
            angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
                return {
                    templateUrl: 'plugins/' + pluginName + '/lldb_cmd.html',
                    link: function(scope, element, attrs) {
                        scope.base_container = $(element);
                    }
                };
            }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile',
                function ($scope, $http, $compile) {
                    var session = $scope.$parent.session;

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
                }
            ]);
        },

        constructor: function() {
            return function() {
                function Plugin() {}

                Plugin.prototype.get_name = function() {
                    return pluginName;
                };

                Plugin.prototype.get_descriptor = function() {
                    return "LLDB Command Line";
                };

                return new Plugin();
            }
        }
    };

    register_plugin(pluginSpec);

    // register commands
    register_command({
        cmd: "lldb",
        session_required: true,
        complete: function(params, _session) {
            return ["[lldb command pass through]"];
        },

        execute: function(params, _session) {
            _session.lldbCmd(params.join(" "), function(data) {
                if(data.result != "") {
                    data.result.split("\n").forEach(function(line) {
                        log(line);
                    });
                } else {
                    log(data);
                }
            });
        }
    });
})();