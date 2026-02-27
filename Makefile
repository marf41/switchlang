all:
	gcc -O2 -o parser main.c

looptest: all
	parser "dbg 1000 98 set 10000000 loop 1- dup 2 2 * drop done drop clk dup 99 get - 98 mmin 99 set 98 get . 4 goto"

fizzbuzz: all
	parser ': mult 1000 ; \
		: fb mod 0 = if mult * + else drop fi ; \
		: fiz dup 1 swap 3 fb ; \
		: buzz dup 2 swap 5 fb ; \
		: iff dup 1 = if ." fizz " fi ; \
		: ifb dup 2 = if ." buzz " fi ; \
		: iffb dup 3 = if ." fizzbuzz " fi ; \
		: .if if . else drop fi ; \
		: fzbz dup mult / iff ifb iffb drop dup 100 < .if ; \
		0 loop 1+ dup dup fiz buzz fzbz cr 100 < done'