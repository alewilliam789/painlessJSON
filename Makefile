file_header = -isystem./include
library_headers = -isystem./lib/include
library_path = -L./lib
libraries = -lhashtable -larena -llinkedlist -lhash 
base_flags = -Wall -pedantic -Werror -Wextra


painless : main.c scanner.c parser.c
	gcc $(file_header) $(library_headers) $(base_flags) -o painless main.c scanner.c parser.c $(library_path) $(libraries)
	
test: json-parser.test.c parser.c scanner.c
	gcc $(file_header) $(library_headers) $(base_flags) -g json-parser.test.c parser.c scanner.c $(library_path) $(libraries) 

library :
	make clean
	- rm ../lib/libjsonparser.so ../lib/include/parser.h ../lib/include/scanner.h
	gcc $(file_header) $(library_headers) -c $(base_flags) -fPIC main.c parser.c scanner.c $(library_path) $(libraries)
	gcc -shared -o libpainlessJSON.so main.o parser.o scanner.o
	- rm main.o scanner.o parser.o
	mv ./libpainlessJSON.so ../lib/
	cp ./include/parser.h ../lib/include
	cp ./include/scanner.h ../lib/include


json:
	bear -- make painless
	- rm a.out

debug-parser:
	gcc $(file_header) $(library_headers) $(base_flags) -g main.c parser.c scanner.c $(library_path) $(libraries) 
	gdb a.out

debug-test: 
	- rm a.out
	make test
	gdb a.out

clean: 
	- rm a.out 
	- rm json-parser
