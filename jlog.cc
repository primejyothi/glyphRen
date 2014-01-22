#include <stdio.h>
#include <stdlib.h>
#include "jlog.hpp"

//! Minimum Log level the program that need to maintain.
int JMINLVL = FATAL | ERROR | WARN | LOG | INFO;

int JLOGLVL = LOG; //!< The log level.
int JLFWIDT = 6; //!< Width for the file name field in log messages.
int JLLWIDT = 4; //!< Width for the line number filed in log messages.

