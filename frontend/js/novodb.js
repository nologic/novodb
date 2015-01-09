if (!String.prototype.startsWith) {
  Object.defineProperty(String.prototype, 'startsWith', {
    enumerable: false,
    configurable: false,
    writable: false,
    value: function (searchString, position) {
      position = position || 0;
      return this.lastIndexOf(searchString, position) === position;
    }
  });
}

String.prototype.replaceAt = function(index, character) {
    return this.substr(0, index) + character + this.substr(index+character.length);
}

function load_js_file(filename){
    var fileref = document.createElement('script');

    fileref.setAttribute("type","text/javascript");
    fileref.setAttribute("src", filename);

    if (typeof fileref != "undefined") {
        document.getElementsByTagName("head")[0].appendChild(fileref);
    }
}

window.novo = angular.module('novodb', ['ui.bootstrap', 'datatables']);

novo.config(function(/*$routeProvider, */$controllerProvider, $compileProvider, $filterProvider, $provide) {
    novo.controllerProvider = $controllerProvider;
    novo.compileProvider    = $compileProvider;
    //novo.routeProvider      = $routeProvider;
    novo.filterProvider     = $filterProvider;
    novo.provide            = $provide;

    // Register routes with the $routeProvider
}).controller("dbg", ['$scope', '$http', '$compile', '$timeout',
	function($scope, $http, $compile, $timeout) {
        window.utils = create_utils($http);
        
        $scope.new_current_session = function() {
            $scope.session = create_ndb_session($http);

            // watch state
            var lastState = undefined;
            var rem = $scope.$watch($scope.session.get_procState, function(new_state) {
                if(lastState == undefined && new_state.state != 0) {
                    // we just loaded or attached;
                    lastState = new_state;
                    log(lastState);

                    $scope.instantiatePlugin('Regview');
                    $scope.instantiatePlugin('Memview');
                    $scope.instantiatePlugin('Insview');
                }
            });

            $scope.session.add_destroy_listener(rem);

            log("new session started");
        };

        $scope.step = function () {
            $scope.session.step();
        };

        $scope.stepOver = function () {
            log("this one doesn't work")
        };

        $scope.continue_proc = function () {
            $scope.session.continue_proc();
        };

        $scope.detach = function() {
            $scope.session.detach(function(data){
                $('.plugin-outer').remove();

                $scope.session.destroy();
                $scope.new_current_session();

                $scope.instantiatePlugin('Loader');

                log("process detached");
            }, log);
        }

        $scope.plugins = [];
        $scope.instantiatePlugin = function(pluginName, params) {
            $timeout(function() {
                $scope.plugins.push({
                    name: pluginName,
                    session: $scope.session,
                    params: params
                });
            });
        };

        // start a session
        $scope.new_current_session();

        register_command({
            cmd: "open",
            complete: function(params) {
                if(params.length == 0) {
                    return ["[plugin name] ..."];
                } else {
                    return window.plugin_specs.map(function(spec) {
                        return spec.get_plugin_name();
                    }).filter(function(name) {
                        return name.startsWith(params[params.length - 1]);
                    });
                }
            },

            execute: function(params) {
                params.forEach(function(pl) {
                    $scope.instantiatePlugin(pl);
                });
            }
        });

        $scope.jQueryLoaded = function() {
            $scope.instantiatePlugin('Loader');
        }
	}
]).directive("ndbPluginsContainer", function($compile) {
    return {
        scope:{
            ndbPluginsContainer:"=" //import referenced model to our directives scope
        },
        link: function(scope, elem, attr, ctrl) {
            scope.$watch('ndbPluginsContainer', function(){ // watch for when model changes
                while(scope.ndbPluginsContainer.length > 0) {
                    var d = scope.ndbPluginsContainer.shift();

                    if (d == undefined) {
                        return;
                    }

                    log("Instantiating: " + d.name);

                    var s = scope.$new(); //create a new scope
                    
                    angular.extend(s, d); //copy data onto it

                    s.moveUp = function() {
                        var prev = compiled.prev();
                        
                        if(prev != undefined) {
                            compiled.insertBefore(prev);
                        }
                    }

                    s.moveDown = function() {
                        var next = compiled.next();

                        if(next != undefined) {
                            compiled.insertBefore(prev);
                        }
                    }

                    s.closePlugin = function() {
                        compiled.remove();
                    }

                    s.moveLeft = function() {
                        console.info("moveup");
                    }

                    s.moveRight = function() {
                        console.info("moveup");
                    }

                    var template = 
                        '<div class="plugin-outer">' +
                        '  <div class="plugin-toolbar noselect">' + 
                        '    <span class="blue">' + d.name + '</span>' +
                        '    <span ng-click="moveUp()" class="glyphicon glyphicon-circle-arrow-up blue" aria-hidden="true"></span>' +
                        '    <span ng-click="moveDown()" class="glyphicon glyphicon-circle-arrow-down blue" aria-hidden="true"></span>' +
                        '    <span ng-click="closePlugin()" class="glyphicon glyphicon-remove-circle red" aria-hidden="true"></span>' +
                        '  </div>' +
                        '  <div class="plugin-container" ndb-Plugin-' + d.name + ' ng-controller="ndbPlugin' + d.name + '">' + 
                        '  </div>' +
                        '</div>'

                    var compiled = $compile(template)(s);

                    elem.append(compiled); // compile template & append
                }
            }, true); //look deep into object
        }
    };
});

