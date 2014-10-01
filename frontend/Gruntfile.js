'use strict';
// # Globbing
// for performance reasons we're only matching one level down:
// 'test/spec/{,*/}*.js'
// use this if you want to recursively match all subfolders:
// 'test/spec/**/*.js'

module.exports = function (grunt) {
    // load all grunt tasks
    require('matchdep').filterDev('grunt-*').forEach(grunt.loadNpmTasks);

    grunt.initConfig({
        bower_concat: {
            all: {
                dest: 'js/all.js'
            }
        }
    });

    require('load-grunt-tasks')(grunt);

    grunt.registerTask('build', [
        'bower_concat'
    ]);
};