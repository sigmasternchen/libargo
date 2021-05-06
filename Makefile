CC       = gcc
CFLAGS   = -std=c99 -Wall -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -g
LD       = gcc
LDFLAGS  = 
AR       = ar
ARFLAGS  = rcs

LEX = flex
YACC = bison
YFLAGS = -y -d

MARSHALLER_GEN = marshaller-gen

A_LIB_NAME = libcson.a
SO_LIB_NAME = libcson.so

OBJS     = obj/base.o obj/parse.o obj/query.o obj/stringify.o obj/marshaller.o
DEPS     = $(OBJS:%.o=%.d)

all: $(A_LIB_NAME) $(SO_LIB_NAME) tests

tests: json-test marshaller-test
	./json-test
	./marshaller-test

$(A_LIB_NAME): CFLAGS += -fPIC
$(A_LIB_NAME): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(SO_LIB_NAME): CFLAGS += -fPIC
$(SO_LIB_NAME): $(OBJS)
	$(LD) -shared -o $@ $^

-include $(DEPS)

obj/%.o: src/%.c obj
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

obj:
	mkdir -p obj

$(MARSHALLER_GEN): marshaller/codegen.c gen/lex.yy.c gen/y.tab.c
	$(CC) $(CFLAGS) -Imarshaller/ -o $@ $^

gen/y.tab.c gen/y.tab.h: marshaller/parser.y gen
	$(YACC) $(YFLAGS) $<
	mv y.tab.c gen/
	mv y.tab.h gen/

gen/lex.yy.c: marshaller/scanner.l gen/y.tab.h gen
	$(LEX) $<
	mv lex.yy.c gen/

gen:
	mkdir -p gen/


json-demo: demo/json.c $(A_LIB_NAME)
	$(CC) $(CFLAGS) -Isrc/ -o $@ $^

marshaller-demo: gen/demo.tab.c demo/marshaller.c $(A_LIB_NAME)
	$(CC) $(CFLAGS) -Isrc/ -Idemo/ -Isrc/ -o $@ $^

gen/demo.tab.c: demo/demo.h $(MARSHALLER_GEN)
	./$(MARSHALLER_GEN) -o $@ $<


json-test: test/json.c $(A_LIB_NAME)
	$(CC) $(CFLAGS) -Isrc/ -o $@ $^

marshaller-test: gen/test.tab.c test/marshaller.c $(A_LIB_NAME)
	$(CC) -g -Itest/ -Isrc/ -o $@ $^

gen/test.tab.c: test/test*.h $(MARSHALLER_GEN)
	./$(MARSHALLER_GEN) -o $@ test/test*.h

clean:
	@echo "Cleaning up..."
	@rm -f obj/*.o
	@rm -f obj/*.d
	@rm -f gen/*.c
	@rm -f $(A_LIB_NAME)
	@rm -f $(SO_LIB_NAME)
	@rm -f $(MARSHALLER_GEN)
	
	@rm -f json-demo
	@rm -f json-test
	
	@rm -f marshaller-demo
	@rm -f marshaller-test
