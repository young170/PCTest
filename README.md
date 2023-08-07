# PCTest
PCTest: Multi-processed Programming Assignment Testers
> PCTest is an assistant program for evaluating C programming courses.

## Build
Build `pctest` using the `Makefile`<br><br>
Example:
```
$ make
gcc -Wall -o pctest.o -c pctest.c -I./include
gcc -Wall -o pctest pctest.o
```

## Run
### Command-Line Interface
Usage:
```
$ pctest -i <testdir> -t <timeout> <solution> <target>
```
* `testdir`: directory path where test inputs are stored
* `timeout`: time limit of program execution
    * accepts integer values from 1 to 10
* `solution`: filename of the correct program source code
* `target`: filename of the student's program source code
