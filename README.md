# Novodb - Deep inspection beyond debugging #

Novodb is a platform for building inspection user interfaces.

A new way to debug software. A plugin architecture is designed to support many use cases by providing customization and extensibility.

## Use cases ##
We aim to support the following use cases.

### Vulnerability analysis

* Ability to overlay partially defined data structures over the memory. 
* Run simulations and provide recording of traces.
* Multiple target debugging

### Capture the flag ###

* Match common patterns of challenges
* Cooler looking UI for browny points at the table

### Memory Forensics ###

* Dynamically analyze what is happening on the host.

## Extensibility ##

To support extensions the architecture support plugins in the following spaces:

* GUI - make it look as you wish with access to the same underlying functionality
* Debugger - provide additional fundamental functionality (i.e. remote agents, etc)
* Utils - provide platform functions (i.e. process listing, etc).
* Analytics - build new analysis tools.