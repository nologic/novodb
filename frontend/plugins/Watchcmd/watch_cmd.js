(function() {
    var pluginName = "Watchcmd";

    var pluginSpec = {
        name: pluginName,
        type: undefined,

        staticInit: function(angular_module) {
            angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
                return {
                    templateUrl: 'plugins/' + pluginName + '/watch_cmd.html',
                    link: function(scope, element, attrs) {
                        scope.base_container = $(element);
                    }
                };
            }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile', 'DTOptionsBuilder', 'DTColumnBuilder',
                function ($scope, $http, $compile, DTOptionsBuilder, DTColumnBuilder) {
                    var session = $scope.$parent.session;

                    var rem = $scope.$watch(session.get_stepCount, function(new_step) {
                        var command = $scope.cmdline;

                        if(command != "") {
                            session.lldbCmdNoStep(command, function(data) {
                                if(data.result != "") {
                                    $scope.output = data.result;
                                } else {
                                    log(data);
                                }
                            });
                        }
                    });

                    session.add_destroy_listener(rem);
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
                    return "Process loader";
                };

                return new Plugin();
            }
        }
    };

    register_plugin(pluginSpec);
})();