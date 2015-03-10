/*

Plugin JS template:
(function() {
    var pluginName = "Loader"; // Must start with upper and continue with lower cases.

    var pluginSpec = {
        name: pluginName,
        type: undefined,

        staticInit: function(angular_module) {
            angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
                return {
                    templateUrl: 'plugins/' + pluginName + '/loader.html',
                    link: function(scope, element, attrs) {
                        scope.base_container = $(element);
                    }
                };
            }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile', 'DTOptionsBuilder', 'DTColumnBuilder',
                function ($scope, $http, $compile, DTOptionsBuilder, DTColumnBuilder) {
                    var session = $scope.$parent.session;

                    // do general angular stuff on per instance basis.
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

*/

