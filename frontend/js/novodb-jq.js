var commands = [];

function register_command(cmd) {
    commands.push(cmd);
}

$( document ).ready(function() {
    $('[data-toggle-tp="tooltip"]').tooltip({
        animated: 'fade',
    });

    $('#footer_terminal').terminal(function (command, term) {
        // execute command!
        var cmdText = $.terminal.splitCommand(command);

        var execCmd = commands.filter(function(cmd) {
            return cmd.cmd == cmdText.name;
        });

        if(execCmd.length > 1) {
            term.echo("Disambiguate: " + execCmd.map(function(cmd) {
                return cmd.cmd;
            }).join(" "));
        } else if(execCmd.length == 1) {
            var execStr = execCmd[0].execute(cmdText.args);

            if(execStr != undefined) {
                term.echo(execStr);
            }
        } else {
            term.echo("Command '" + cmdText.name + "' not found");
        }
    }, {
            greetings: "[[i;red;]Welcome to Novodb. Enjoy your debugging experience!]",
            keydown: function(event, term) {
            function longest_start(arr, hint) {
                if(hint == undefined) {
                    hint = "";
                }

                var shortest = arr.reduce(function(acc, testVal) {
                    return Math.min(testVal.length, acc);
                }, arr[0].length);

                if(shortest < hint) {
                    return hint;
                } else {
                    while(hint.length < shortest) {
                        for(var i = 0; i < arr.length; i++) {
                            if(!arr[i].startsWith(hint)) {
                                // we hit first, not common item.
                                return hint.substring(0, hint.length - 1);
                            }
                        }

                        hint = arr[0].substring(0, hint.length + 1);
                    }

                    return hint;
                }
            }

            if (event.keyCode == 9) {
                // tab completion
                var cmdText = $.terminal.splitCommand(term.get_command());
        
                var execCmd = commands.filter(function(cmd) {
                    return cmd.cmd.indexOf(cmdText.name) == 0;
                });

                if(execCmd.length > 1) {
                    term.echo(execCmd.map(function(cmd) {
                        return cmd.cmd;
                    }).join(" "));
                } else if(execCmd.length == 1) {
                    if(cmdText.args.length == 0) {
                        term.set_command(execCmd[0].cmd + " ");
                    }

                    var compl = execCmd[0].complete(cmdText.args);

                    if(compl != undefined) {
                        function proc_completes(arr) {
                            var apply_guess = false;
                            if(cmdText.args.length == 0){
                                // print out the "help"
                                term.echo(execCmd[0].cmd + ": " + arr.join(" "));
                            } else if(arr.length == 1) {
                                cmdText.args[cmdText.args.length - 1] = arr[0];

                                term.set_command(execCmd[0].cmd + " " + cmdText.args.join(" "));
                            } else if(arr.length < 10) {
                                apply_guess = true;
                                term.echo(execCmd[0].cmd + ":");
                                term.echo(arr.join(" "));
                            } else {
                                apply_guess = true;
                                arr.forEach(function(c) {
                                    term.echo(" " + c);
                                });
                            }

                            if(apply_guess && arr.length > 0) {
                                cmdText.args[cmdText.args.length - 1] = longest_start(arr);

                                term.set_command(execCmd[0].cmd + " " + cmdText.args.join(" "));
                            }
                        }

                        if(typeof compl.then === 'function') {
                            compl.then(proc_completes);
                        } else {
                            proc_completes(compl);
                        }
                    }
                } else {
                    term.echo("Command '" + cmdText.name + "' not found");
                }

                // Tells the terminal to not handle the tab key
                return false;
            }
        }
    });

    window.output = function(txt) {
        $('#footer_terminal').terminal().echo(txt);
    }

    angular.element('#dbg').scope().jQueryLoaded();
});

function getStackTrace() {
  var obj = {};
  Error.captureStackTrace(obj, getStackTrace);
  return obj.stack;
};