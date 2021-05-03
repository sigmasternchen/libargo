LEX = flex
YACC = bison
YFLAGS = -y -d
CC = gcc

BIN_NAME = marshaller

marshaller: gen/lex.yy.c gen/y.tab.c
	$(CC) -Isrc/ -o $@ $^

gen/y.tab.c gen/y.tab.h: src/parser.y
	$(YACC) $(YFLAGS) $<
	mv y.tab.c gen/
	mv y.tab.h gen/

gen/lex.yy.c: src/scanner.l gen/y.tab.h
	$(LEX) $<
	mv lex.yy.c gen/

clean:
	rm -f gen/* $(BIN_NAME)

