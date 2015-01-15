load_plugin(function() {
    var _instance = undefined;
    var session = undefined;
    var pluginName = "Watchcmd";

    function Plugin() {}

    Plugin.prototype.attach_ui = function(angular_module, div_container_jq) {
        angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
            return {
                templateUrl: 'plugins/' + pluginName + '/watch_cmd.html',
                link: function(scope, element, attrs) {
                    scope.base_container = $(element);
                }
            };
        }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile',
            function ($scope, $http, $compile) {
                _instance.set_session($scope.$parent.session);

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