LEX = flex
YACC = bison
YFLAGS = -y -d
CC = gcc
LD = gcc

BIN_NAME = marshaller

marshaller: gen/lex.yy.c gen/y.tab.c
	$(CC) -o $@ $^

gen/y.tab.c gen/y.tab.h: src/parser.y
	$(YACC) $(YFLAGS) $<
	mv src/y.tab.c gen/
	mv src/y.tab.h gen/

gen/lex.yy.c: src/scanner.l gen/y.tab.h
	$(LEX) $<
	mv src/lex.yy.c gen/

clean:
	rm -f gen/* $(BIN_NAME)

