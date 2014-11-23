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

window.novo = angular.module('novodb', ['ui.bootstrap']);

novo.config(function(/*$routeProvider, */$controllerProvider, $compileProvider, $filterProvider, $provide) {
    novo.controllerProvider = $controllerProvider;
    novo.compileProvider    = $compileProvider;
    //novo.routeProvider      = $routeProvider;
    novo.filterProvider     = $filterProvider;
    novo.provide            = $provide;

    // Register routes with the $routeProvider
}).controller("dbg", ['$scope', '$http', '$compile', '$timeout',
	function($scope, $http, $compile, $timeout) {
        var session = create_ndb_session($http);

		$scope.getThreads = function() {
			session.getThreads(function(resp) {
				$scope.thread_list = JSON.stringify(resp);
			});
		};

		$scope.getModules = function() {
			session.getModules(function(data) {
				$scope.module_output = data;
			});
		};

		$scope.listSymbols = function() {
            session.getSymbols(0, function(data) {
				$scope.symbols_output = data;
			});
		};

        $scope.getFrames = function(thread_ind) {
            session.getFrames(thread_ind, function(data) {
                $scope.frames_list = JSON.stringify(data, null, "\t");
            });
        };

        $scope.plugins = [];
        $scope.instantiatePlugin = function(pluginName, params) {
            $timeout(function() {
                $scope.plugins.push({
                    name: pluginName,
                    session: session,
                    params: params
                });
            });
        };

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

        // watch state
        var lastState = undefined;
        $scope.$watch(session.get_procState, function(new_state) {
            if(lastState == undefined && new_state.state != 0) {
                // we just loaded or attached;
                lastState = new_state;
                log(lastState);

                $scope.instantiatePlugin('Regview');
                $scope.instantiatePlugin('Memview');
                $scope.instantiatePlugin('Insview');
            }
        });
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

                    var template = '<div class="col-md-4 column" ndb-Plugin-' + d.name + ' ng-controller="ndbPlugin' + d.name + '"></div>';
                    elem.append($compile(template)(s)); // compile template & append
                }
            }, true); //look deep into object
        }
    };
});

