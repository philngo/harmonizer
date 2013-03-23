import: import.c
	clang -ggdb -O0 -Qunused-arguments -std=c99 -Wall -Werror -c -o import.o import.c -I/usr/local/include/libxml2/ -lm -lxml2 
	clang -ggdb -O0 -Qunused-arguments -std=c99 -Wall -Werror -c -o musicxml.o musicxml.c -I/usr/local/include/libxml2/ -lm -lxml2 -lgsl -lgslcblas
	clang -ggdb -O0 -Qunused-arguments -std=c99 -Wall -Werror -o import import.o musicxml.o -I/usr/local/include/libxml2/ -lm -lxml2 -I/usr/local/include -L/usr/local/lib -lm -lgsl -lgslcblas
