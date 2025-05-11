CC=gcc
CFLAGS=-Wall

all: parser

parser: out/lex.yy.c out/parser.tab.c
	$(CC) $(CFLAGS) -o parser out/lex.yy.c out/parser.tab.c -ly

out/parser.tab.c out/parser.tab.h: src/parser.y | out
	bison -d -o out/parser.tab.c src/parser.y

out/lex.yy.c: src/lexer.l out/parser.tab.h | out
	flex -o out/lex.yy.c src/lexer.l

out:
	mkdir -p out

clean:
	rm -f parser out/parser.tab.c out/parser.tab.h out/lex.yy.c
	rmdir out