(function() {
    var pluginName = "Breakpoint";

    var pluginSpec = {
        name: pluginName,
        type: undefined,

        staticInit: function(angular_module) {
            angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
                return {
                    templateUrl: 'plugins/' + pluginName + '/breakpoint.html',
                    link: function(scope, element, attrs) {
                        scope.base_container = $(element);
                    }
                };
            }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile',
                function ($scope, $http, $compile) {
                    var session = $scope.$parent.session;

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
        },

        constructor: function() {
            return function() {
                function Plugin() {}

                Plugin.prototype.get_name = function() {
                    return pluginName;
                };

                Plugin.prototype.get_descriptor = function() {
                    return "Breakpoints";
                };

                return new Plugin();
            }
        }
    };

    register_plugin(pluginSpec);
})();