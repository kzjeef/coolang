#!/bin/bash
make parser
if [ $? -ne 0 ];
then
	echo "build error".
	exit
fi
cat grading/$1.test
echo ----------------
cd grading
../lexer $1.test > lexer.out 
../parser < lexer.out 2>&1 > parser.out 
echo =============
#cat $1.test.out 

echo "parser.out"  " | "    "$1.test.out"
pr -mt parser.out $1.test.out

echo ----------------
diff -u $1.test.out parser.out
if [ $? -eq 0 ];
then
	echo pass;
fi
