LEX = flex
YACC = bison
YFLAGS = -y -d
CC = gcc

BIN_NAME = marshaller-gen

$(BIN_NAME): src/codegen.c gen/lex.yy.c gen/y.tab.c
	$(CC) -Isrc/ -o $@ $^

gen/y.tab.c gen/y.tab.h: src/parser.y gen
	$(YACC) $(YFLAGS) $<
	mv y.tab.c gen/
	mv y.tab.h gen/

gen/lex.yy.c: src/scanner.l gen/y.tab.h gen
	$(LEX) $<
	mv lex.yy.c gen/

gen:
	mkdir -p gen/

clean:
	rm -f gen/* $(BIN_NAME)
