(function() {
    var pluginName = "Insview";

    var pluginSpec = {
        name: pluginName,
        type: undefined,

        staticInit: function(angular_module) {
            angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
                return {
                    templateUrl: 'plugins/' + pluginName + '/insview.html',
                    link: function(scope, element, attrs) {
                        scope.base_container = $(element);
                    }
                };
            }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile',
                function ($scope, $http, $compile) {
                    var session = $scope.$parent.session;

                    $scope.readInstructions = function (_address) {
                        session.readInstructions(_address, 20, function(data) {
                            $scope.inst_addr = _address;
                            $scope.inst_output = data;
                        });
                    };

                    $scope.$watch(session.get_currentPc, function(newVal) {
                        if(newVal != undefined) {
                            $scope.readInstructions(newVal.value);
                        }
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
                    return "Instruction Viewer";
                };

                return new Plugin();
            }
        }
    };

    register_plugin(pluginSpec);
})();