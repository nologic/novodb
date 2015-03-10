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

    register_command({
        cmd: "ida",
        quick_help: "Interact with IDAPro",
        complete: function(params, _session) {
            if(params.length == 0) {
                return ["connect"];
            }

            return ["no help"];
        },

        execute: function(params, _session) {
            var subcmd = params.shift();
            var idaapi = get_api("idarest");

            log(idaapi);

            if(idaapi === undefined) {
                log("idarest api not defined.");

                return;
            }

            if(subcmd === "connect") {
                var url = params.shift();

                var conn = idaapi.connect(url);
                conn.segments(log);
            }
        }
    });

    register_plugin(pluginSpec);
})();

(function(){
    function default_err(resp) {
        log(resp.status + " " + resp.statusText);

        console.info(resp);
    }

    function default_success(resp) {
        log(resp.status + " " + resp.statusText);

        console.info(resp);
    }

    function extract_data(func) {
        return function(resp){
            if(func == undefined) {
                return resp.data;
            } else if(typeof(func) == "function") {
                return func(resp.data);
            } else {
                // we actually have an array of functions to exec.
                return func.reduce(function(acc, f) {
                    if(f != undefined) {
                        return f(resp.data);
                    }
                }, undefined);
            }
        }
    }

    function url_get_passthrough(url, path, params, f_success, f_fail) {
        var $http = angular.element('html').injector().get('$http');

        call_method = "JSONP";

        return $http({
            url: url + "/" + path + "?callback=JSON_CALLBACK",
            method: call_method,
            params: params
        }).then(function (resp) {
            if (f_success == undefined) {
                f_success = default_success;
            }
            
            if(call_method == "JSONP" && 'data' in resp) {
                // mask it to make it look like we did a regular GET.
                resp.status = resp.data.code;
                resp.statusText = resp.msg;
                resp.data = resp.data.output;
            }
            
            if(call_method == "JSONP" && resp.status != 200) {
                if (f_fail == undefined) {
                    f_fail = default_err;
                }

                return f_fail(resp);
            } else {
                return f_success(resp);
            }

            return f_success(resp);
        }, function (resp) {
            if (f_fail == undefined) {
                f_fail = default_err;
            }

            if(call_method == "JSONP" && 'data' in resp) {
                resp.status = resp.data.code;
                resp.statusText = resp.data.msg;
                resp.data = resp.data.output;
            }

            return f_fail(resp);
        });
    }




    function API(url) {
        this.url = "http://" + url + "/ida/api/v1.0";
    };

    API.prototype = {
        segments: function(callback) {
            url_get_passthrough(this.url, "segments", {}, callback);
        }
    };

    register_api("idarest", {
        connect: function(url) {
            return new API(url);
        }
    });
})();