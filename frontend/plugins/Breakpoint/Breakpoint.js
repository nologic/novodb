load_plugin(function() {
    var _instance = undefined;
    var session = undefined;
    var pluginName = "Breakpoint";

    function Plugin() {}

    Plugin.prototype.attach_ui = function(angular_module, div_container_jq) {
        angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
            return {
                templateUrl: 'plugins/' + pluginName + '/breakpoint.html',
                link: function(scope, element, attrs) {
                    scope.base_container = $(element);
                }
            };
        }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile',
            function ($scope, $http, $compile) {
                _instance.set_session($scope.$parent.session);

                $scope.breakpoints = [];

                $scope.setBreakpoint = function(symbol) {
                    var type = "symbol";
                    if(symbol.indexOf("0x") == 0) {
                        type = "address"; 
                    }

                    session.addBreakpoint(symbol, type, function(data) {
                        $scope.breakpoints.push(data);
                        log(data.description);
                    });
                };

                $scope.locations_string = function(locations) {
                    return locations.map(function(loc) {
                        return loc.load_address;
                    }).join(" ");
                };

                $scope.description_string = function(description) {
                    return description.split(",").filter(function(item) {
                        return item.indexOf("name") >= 0;
                    }).join(" ");
                }
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