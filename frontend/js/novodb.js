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

window.path_to_url = function(plugin, path) {
    return plugin + "://" + path;
};
window.call_method = "GET";

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
        
        $scope.connect_agent_str = "localhost:4070";

        $scope.connect_agent = function(con_agent) {
            var host = con_agent.split(":");
            var port = host[1];

            host = host[0];

            if(port === undefined) {
                port = "4070";
            }

            window.path_to_url = function(plugin, path) {
                return "http://" + host + ":" + port + "/" + plugin + 
                       "/" + path + "?callback=JSON_CALLBACK";
            };

            window.call_method = "JSONP";

            $('#connect_screen').modal('hide');

            // start a session
            $scope.new_current_session();
            $scope.instantiatePlugin('Loader');
        };

        $scope.new_current_session = function() {
            $scope.session = create_ndb_session($http);
            window.getCurrentSession = function() {
                return $scope.session;
            };

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
                $scope.session.destroy();
                $scope.new_current_session();

                $scope.instantiatePlugin('Loader');

                log("process detached");
            });
        }

        window.myLayout = new GoldenLayout({
            content:[{
                type: 'row',
                isClosable: false,
                content: []
            }]
        }, $('#plugins_container'));

        window.myLayout.registerComponent( 'angularModule', function( container, state ) {
            state.store_item(container);

            element = container.getElement();
            element.html( state.template );

            angular
                .module( state.module )
                .value( 'container', container )
                .value( 'state', state );

            angular.bootstrap( element[ 0 ], [ state.module ] );
        });

        window.myLayout.init();

        function resizedw(){
            window.myLayout.updateSize();
        }

        var doit;
        window.onresize = function(){
          clearTimeout(doit);
          doit = setTimeout(resizedw, 100);
        };

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

        if(window.cef_embed != undefined) {
            // start a session
            $scope.new_current_session();
        }

        register_command({
            cmd: "open",
            quick_help: "[plugin name] ...",
            complete: function(params) {
                var startStr = (params.length > 0)?params[params.length - 1]:"";
                var ret = [];

                for(name in window.loaded_plugins) {
                    if(name.startsWith(startStr)) {
                        ret.push(name);
                    }
                }

                return ret;
            },

            execute: function(params) {
                params.forEach(function(pl) {
                    $scope.instantiatePlugin(pl);
                });
            }
        });

        $scope.jQueryLoaded = function() {
            if(window.cef_embed != undefined) {
                $scope.instantiatePlugin('Loader');
            }
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

                    s.closePlugin = function() {
                        window.myLayout.removeChild(s.window_id);
                    }

                    var template = 
                        '<div class="plugin-container" ndb-Plugin-' + d.name + 
                        ' ng-controller="ndbPlugin' + d.name + '"></div>';

                    var compiled = $compile(template)(s);

                    var contentItem = null;

                    window.myLayout.root.contentItems[0].addChild({
                        width: 20,
                        title: d.name,
                        type: 'component',
                        componentName: 'angularModule',
                        componentState: {
                            module: 'novodb',
                            template: compiled,
                            store_item: function(item) {
                                // a very convoluted way of obtaining the container item
                                // called in the angularModule function.
                                contentItem = item;
                            }
                        }
                    });

                    s.closePlugin = function() {
                        contentItem.close();
                    };
                }
            }, true); //look deep into object
        }
    };
});

