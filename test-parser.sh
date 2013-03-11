
rm ./parser
make parser
cat grading/$1.test
echo ----------------
cd grading
../lexer $1.test > lexer.out 
../parser < lexer.out | tee parser.out
echo =============
cat $1.test.out 

echo ----------------
diff -u $1.test.out parser.out
