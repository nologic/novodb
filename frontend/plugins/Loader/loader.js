load_plugin(function() {
    // these variable are "static" (ie. same across instances)
    var _instance = undefined;
    var session = undefined;
    var pluginName = "Loader";

    function Plugin() {}

    Plugin.prototype.attach_ui = function(angular_module, div_container_jq) {
        angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
            return {
                templateUrl: 'plugins/' + pluginName + '/plugin.html',
                link: function(scope, element, attrs) {
                    scope.base_container = $(element);
                }
            };
        }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile',
            function ($scope, $http, $compile) {
                _instance.set_session($scope.$parent.session);
                
                $scope.load_processes = function() {
                    $http.get('util://list/proc').then(function (data) {
                        $scope.processes = data.data.processes.map(function(proc) {
                            // load by path
                            var slashIndex = proc.path.lastIndexOf('/');
                            
                            proc.name = proc.path.substring(slashIndex + 1, proc.path.length);
                            proc.path = proc.path.substring(0, slashIndex + 1);

                            return proc;
                        });
                    });
                };

                $scope.attach = function(proc) {
                    session.attach(proc.pid, function() {
                        $('#controls_bar').show();

                        $scope.base_container.remove();

                        log("Attaced to " + proc.pid + ":" + proc.name);
                    });
                };

                $scope.load_processes();
            }
        ]);
    };

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