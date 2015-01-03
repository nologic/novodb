load_plugin(function() {
    var _instance = undefined;
    var session = undefined;
    var pluginName = "Symbolics";

    function Plugin() {}

    Plugin.prototype.attach_ui = function(angular_module, div_container_jq) {
        angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
            return {
                templateUrl: 'plugins/' + pluginName + '/symbolics.html',
                link: function(scope, element, attrs) {
                    scope.base_container = $(element);
                }
            };
        }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile',
            function ($scope, $http, $compile) {
                _instance.set_session($scope.$parent.session);

                $scope.moduleList = {
                    selected: ""
                };

                $scope.load_modules = function() {
                    session.getModules(function(data) {
                        $scope.modules = data.modules;
                    });
                }

                $scope.load_symbols = function() {
                    session.getSymbols(0, function(data) {
                        $scope.symbols = data.symbols;
                    });
                };

                $scope.selected_module = function() {
                    log($scope.moduleList);
                };

                $scope.clicked_module = function(mod) {
                    session.getSymbols(mod.index, function(data) {
                        $scope.module_data = data.symbols;
                    });
                    // because Angular sucks with <select>
                    var modDiv = $scope.base_container.find(".list_mod_" + mod.index);

                    if(modDiv.hasClass("modSelected")) {
                        mod.selected = false;
                        modDiv.removeClass("modSelected");
                    } else {
                        mod.selected = true;
                        modDiv.addClass("modSelected");
                    }
                };

                $scope.load_symbols();
                $scope.load_modules();
            }
        ]);
    }

    Plugin.prototype.set_session = function(new_session) {
        session = new_session;
    };

    Plugin.prototype.get_plugin_name = function() {
        return pluginName;
    };

    _instance = new Plugin();

    return _instance;
}());