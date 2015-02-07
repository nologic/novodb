(function() {
    var pluginName = "Symbolics";

    var pluginSpec = {
        name: pluginName,
        type: undefined,

        staticInit: function(angular_module) {
            angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
                return {
                    templateUrl: 'plugins/' + pluginName + '/symbolics.html',
                    link: function(scope, element, attrs) {
                        scope.base_container = $(element);
                    }
                };
            }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile',
                function ($scope, $http, $compile) {
                    var session = $scope.$parent.session;

                    $scope.moduleList = {
                        selected: ""
                    };

                    $scope.search_symbols = function(sym_search_re) {
                        //var mi = $($scope.base_container.find(".list_mod.modSelected")[0]).attr("module-index");/*.each(function(i, r) {
                        //    console.info($(r).attr("module-index")); 
                        //});*/
                        
                        var mi = $('#selected_module').val();

                        session.searchSymbols(mi, 0, 999999, 100, sym_search_re, function(data){
                            $scope.symbol_list = data.symbols;
                        }, log);
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
                        /*session.getSymbols(mod.index, function(data) {
                            $scope.module_data = data.symbols;
                        });*/
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