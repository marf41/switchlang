all:
	gcc -O2 -o parser main.c

looptest: all
	parser "dbg 1000 98 set 10000000 loop 1- dup 2 2 * drop done drop clk dup 99 get - 98 mmin 99 set 98 get . 4 goto"
