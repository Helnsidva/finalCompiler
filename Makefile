compiler.exe : main.o scanner.o parser.o codeGenerator.o
	gcc -o compiler main.o scanner.o parser.o codeGenerator.o

main.o : main.c
	gcc -std=c11 -pedantic -Wall -Wextra -c -o main.o main.c

scanner.o : scanner.c
	gcc -std=c11 -pedantic -Wall -Wextra -c -o scanner.o scanner.c

parser.o : parser.c
	gcc -std=c11 -pedantic -Wall -Wextra -c -o parser.o parser.c

codeGenerator.o : codeGenerator.c
	gcc -std=c11 -pedantic -Wall -Wextra -c -o codeGenerator.o codeGenerator.c