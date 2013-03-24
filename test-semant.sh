#!/bin/bash
make semant
if [ $? -ne 0 ];
then
	echo "build error".
	exit
fi
cat grading/$1.test
echo ----------------
cd grading
#../lexer $1.test > lexer.out 
#../parser < lexer.out 2>&1 > parser.out
../lexer $1.test | ../parser | ../semant > semant.out
echo =============
#cat $1.test.out 

echo "semant.out"  " | "    "$1.test.out"
pr -mt semant.out $1.test.out

echo ----------------
diff -u $1.test.out semant.out  
if [ $? -eq 0 ];
then
	echo pass;
fi
