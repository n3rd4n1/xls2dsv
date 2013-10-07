xls2dsv
=======

xls2dsv is a utility to convert Microsoft Excel (XLS) files to Delimiter-Separated Values (DSV) file. It currently supports BIFF8 (97 - 2007) excel files and can output pure ASCII, UTF-16BE or UTF-8 encoded files.

This program was originally created to help in the conversion process of Excel files to code header files on automated builds.

Building
--------

Executing make will build the utility as "xls2dsv". Makefile can be modified to specify a different compiler and compile flags.


Usage
-----

`./xls2dsv -help`
