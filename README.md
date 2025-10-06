# LispInterpreter
/root
  ├── makefile - Defines the make commands to build and test the interpreter
  ├── README - Directory structure and list of make commands
  ├── documentaion_submission.txt - Information on testing plan/methods, results, and current limitations
  └── src/
       ├── interpreter.c - Code for all of the functions
       ├── interpreter.h - Header file for all functions
       ├── main.c - Contains main
       └── tests.c - Code for testing

# Build Process 
To build REPL:
    make lisp

To run REPL:
    make run

To build and run REPL:
    make

To build and run tests (outputs to a file and terminal):
    make test

To cleanup test:
    make clean

If you want to run the tests mutliply times you have to clean after every 'make test' run

To exit repl, enter (exit)