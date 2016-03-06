#!/bin/bash
echo "make sure you are in the root directory of the project"
echo "testing test makefile. Expected output: \"(true)\""
test makefile
echo "testing [makefile]. Expected output: \"(true)\""
[makefile]
echo "testing test -e makefile. Expected output \"(true)\""
test -e makefile
echo "testing [-e makefile]. Expected output \"(true)\""
[-e makefile]
echo "testing [-f makefile]. Expected output \"(true)\""
[-f makefile]
echo "testing [-d makefile]. Expected output \"(false\""
[-d makefile]
echo "testing test -d src. Expected output: \"(true)\""
test -d src
echo "testing test asd;flafs;as. Expected output: \"(false)\""
test asd;flafs;as
echo "run ./parenthesis_command.sh to see test commands with parenthesis and multiple commands"
~                             
