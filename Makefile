all:
	gcc -O2 -o parser main.c

looptest: all
	parser "dbg 1000 98 set 10000000 loop 1- dup 2 2 * drop done drop clk dup 99 get - 98 mmin 99 set 98 get . 4 goto"

fizzbuzz: all
	parser ': fb mod 0 = if 100 * + else drop fi ; : fiz dup 1 swap 3 fb ; : buzz dup 2 swap 5 fb ; : fzbz dup 100 / dup dup 1 = if ." fizz " fi 2 = if ." buzz " fi 3 = if ." fizzbuzz " fi dup 100 < if . else drop fi ; 0 loop 1+ dup dup fiz buzz fzbz cr 100 < done'