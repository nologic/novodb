load_plugin(function() {
    // these variable are "static" (ie. same across instances)
    var _instance = undefined;
    var session = undefined;
    var pluginName = "Memview";

    function MemView() {}

    MemView.prototype.attach_ui = function(angular_module, div_container_jq) {
        angular_module.directive('ndbPlugin' + pluginName, ['$compile', function ($compile) {
            return {
                templateUrl: 'plugins/' + pluginName + '/plugin.html',
                link: function(scope, element, attrs) {
                    scope.base_container = $(element);
                }
            };
        }]).controller("ndbPlugin" + pluginName, ['$scope', '$http', '$compile',
            function ($scope, $http, $compile) {
                _instance.set_session($scope.$parent.session);
                
                $scope.readMemory = function (_address) {
                    if(_address == undefined || _address === "") {
                        return;
                    }

                    session.readMemory(_address, 4096, function (data) {
                        $scope.read_base = parseInt(_address, 16);
                        $scope.memory_output = data;

                        // maybe not the best way to do this, but for now will do.
                        data_bytes = data.block.match(/.{1,2}/g);

                        if($scope.base_container.find(".mem_bytes").children().length == 0) {
                            initialize(20, 16);
                        }

                        $scope.base_container.find(".mem_bytes span").each(function(offset) {
                            $(this).html(data_bytes[offset]);
                        });

                        $scope.base_container.find(".mem_interpret span").each(function(offset) {
                            $(this).html(interpret_char(data_bytes[offset]));
                        });

                        var parsed_addr = parseInt(_address, 16);
                        $scope.base_container.find(".hex_addr").each(function(offset) {
                            $(this).html( (parsed_addr + (offset * 16)).toString(16)  + ": ");
                        });
                    });
                };

                $scope.$watch(session.get_stepCount, function(newVal) {
                    $scope.readMemory($scope.read_mem_addr);
                });

                function interpret_char(c) {
                    var cint = parseInt(c, 16);

                    if(cint == 0x20) {
                        return "&nbsp";
                    }

                    if(cint > 31 && cint < 127) {
                        return String.fromCharCode(cint);
                    }

                    return ".";
                }

                function initialize(displayRows, displayWidth) {
                    //initialize byte views
                    var byte_container = $scope.base_container.find(".mem_bytes");
                    var addr_container = $scope.base_container.find(".mem_addrs");
                    var intr_container = $scope.base_container.find(".mem_interpret");

                    for(var r = 0, offset = 0; r < displayRows; r++) {
                        (function(row) {
                            var rowClass = "row" + row;
                            var rowbytes = $('<div class="' + rowClass + '"></div>');
                            var rowinter = $('<div class="' + rowClass + '"></div>');

                            byte_container.append(rowbytes);
                            intr_container.append(rowinter);

                            rowinter.append("&nbsp;");
                            rowinter.append("&nbsp;");

                            addr_container.append('<div class="' + rowClass + '"><span class="hex_addr" offset="' + offset + '"></span></div>');

                            for(var c = 0; c < displayWidth; c++, offset++) {
                                if(c != 0 && c % 8 == 0) {
                                    rowbytes.append("&nbsp;");
                                    rowinter.append("&nbsp;");
                                }

                                rowbytes.append('<span class="hex_byte byte_val" offset="' + offset + '"></span>');
                                rowinter.append('<span class="intr_byte byte_val" offset="' + offset + '"></span>');
                            }

                            // for highlighting rows.
                            var addr_row = addr_container.find('.' + rowClass);
                            addr_row.hover(function(){
                                $scope.base_container.find('.' + rowClass).css('background-color', '#C0DFFF');
                            }, function() {
                                $scope.base_container.find('.' + rowClass).css('background-color', 'inherit');
                            });

                            // for highligting offsets.
                            var bytes = $scope.base_container.find(".byte_val");
                            bytes.hover(function(){
                                var _offset = $(this).attr('offset');
                                $scope.base_container.find('span[offset=' + _offset + ']').css('background-color', 'lightgray');
                            }, function() {
                                var _offset = $(this).attr('offset');
                                $scope.base_container.find('span[offset=' + _offset + ']').css('background-color', 'inherit');
                            });
                        })(r);
                    }

                    // Click on/modificaton
                    $scope.base_container.find('.hex_byte').click(function(e) {
                        var hex_span = $(this);
                        var _offset = hex_span.attr('offset');
                        var intr_span = $scope.base_container.find('span[offset=' + _offset + '].intr_byte');

                        if(hex_span.hasClass('hex_byte_edit')) {
                            hex_span.removeClass('hex_byte_edit');
                        } else {
                            $scope.base_container.find('.hex_byte_edit').removeClass('hex_byte_edit');
                            hex_span.addClass('hex_byte_edit');
                            intr_span.addClass('hex_byte_edit');
                        }

                        return false;
                    });

                    byte_container.keypress(function(e) {
                        var kc = e.keyCode;

                        if((kc >= 48 && kc <= 57) || (kc >= 97 && kc <= 102)) {
                            // a hex number
                            var newval = String.fromCharCode(kc);
                            var inedit = $scope.base_container.find('.hex_byte_edit.hex_byte');
                            var _offset = parseInt(inedit.attr('offset'));

                            var eoff = inedit.attr('eoff');

                            if(eoff == undefined) {
                                eoff = 0;
                            } else {
                                eoff = parseInt(eoff);
                            }

                            inedit.html(inedit.html().replaceAt(eoff, newval));
                            session.writeByte(($scope.read_base + _offset).toString(16), parseInt(inedit.html(), 16).toString(), undefined, log);
                            intr_container.find(".hex_byte_edit").html(interpret_char(inedit.html()));
                            eoff += 1;

                            if(eoff < inedit.html().length) {
                                inedit.attr('eoff', eoff);
                            } else {
                                inedit.removeAttr('eoff');

                                // unselect current
                                $scope.base_container.find('.hex_byte_edit').removeClass('hex_byte_edit');

                                // select next
                                var nextBytes = $scope.base_container.find('span[offset=' + (_offset + 1) + ']')
                                nextBytes.addClass('hex_byte_edit');
                            }
                        }
                    });
                }
            }
        ]);
    };

    MemView.prototype.set_session = function(new_session) {
        session = new_session;
    };

    MemView.prototype.set_sessions = function(all_sessions) {

    };

    MemView.prototype.get_plugin_name = function() {
        return pluginName;
    };

    _instance = new MemView();

    return _instance;
}());