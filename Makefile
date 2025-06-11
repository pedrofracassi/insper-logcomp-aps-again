CC=gcc
CFLAGS=-Wall -I.
LLVM_CONFIG=llvm-config
LLVM_CFLAGS=$(shell $(LLVM_CONFIG) --cflags 2>/dev/null || echo "")
LLVM_LDFLAGS=$(shell $(LLVM_CONFIG) --ldflags --libs core 2>/dev/null || echo "")

all: parser stencil-run

parser: out/lex.yy.o out/parser.tab.o out/ast.o out/codegen.o
	$(CC) $(CFLAGS) -o parser out/lex.yy.o out/parser.tab.o out/ast.o out/codegen.o -ly

out/lex.yy.o: out/lex.yy.c out/parser.tab.h
	$(CC) $(CFLAGS) -c out/lex.yy.c -o out/lex.yy.o

out/parser.tab.o: out/parser.tab.c
	$(CC) $(CFLAGS) -c out/parser.tab.c -o out/parser.tab.o

out/ast.o: src/ast.c src/ast.h
	$(CC) $(CFLAGS) -c src/ast.c -o out/ast.o

out/codegen.o: src/codegen.c src/codegen.h src/ast.h
	$(CC) $(CFLAGS) -c src/codegen.c -o out/codegen.o

out/runtime.o: src/runtime.c src/runtime.h
	$(CC) $(CFLAGS) -c src/runtime.c -o out/runtime.o

out/main.o: src/main.c src/runtime.h
	$(CC) $(CFLAGS) -c src/main.c -o out/main.o

# Build executable that can run stencil programs
stencil-run: out/runtime.o out/main.o test.ll
	clang -o stencil-run out/runtime.o out/main.o test.ll

# Generate LLVM IR from stencil source
%.ll: %.stencil parser
	./parser < $< > $@

out/parser.tab.c out/parser.tab.h: src/parser.y | out
	bison -d -o out/parser.tab.c src/parser.y

out/lex.yy.c: src/lexer.l out/parser.tab.h | out
	flex -o out/lex.yy.c src/lexer.l

out:
	mkdir -p out

clean:
	rm -f parser stencil-run out/*.o out/parser.tab.c out/parser.tab.h out/lex.yy.c *.ll
	rmdir out 2>/dev/null || true