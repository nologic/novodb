load_plugin(function(angular_module) {
    angular_module.directive('ndbPluginMemview', ['$compile', function($compile) {
        /*return {
         restrict: 'E',
         scope: { 'plugin': '@' },
         link: function(scope, element) {
         var template = '<h1>hello</h1>',
         compiled = $compile(template)(scope);

         element.append(compiled);
         }
         }*/

        return {
            templateUrl: 'plugins/memview/plugin.html'
        };
    }]).controller("ndbPluginmemview", ['$scope', '$http', '$compile',
        function($scope, $http, $compile) {
            console.info("Controller :", $scope);
        }
    ]);
});