CC       = gcc
CFLAGS   = -std=c99 -Wall -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -g
LD       = gcc
LDFLAGS  = 
AR       = ar
ARFLAGS  = rcs

BIN_NAME = demo
A_LIB_NAME = libcson.a
SO_LIB_NAME = libcson.so

OBJS     = obj/base.o obj/parse.o obj/query.o obj/stringify.o
DEPS     = $(OBJS:%.o=%.d)

all: $(A_LIB_NAME) $(SO_LIB_NAME) $(BIN_NAME)


$(BIN_NAME): obj/demo.o $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(A_LIB_NAME): CFLAGS += -fPIC
$(A_LIB_NAME): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(SO_LIB_NAME): CFLAGS += -fPIC
$(SO_LIB_NAME): $(OBJS)
	$(LD) -shared -o $@ $^

test: obj/test.o $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

-include $(DEPS)

obj/%.o: src/%.c obj
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

obj:
	@mkdir -p obj

marshaller-demo: marshaller/gen/demo.tab.c marshaller/demo/demo.c marshaller/lib/marshaller.c $(A_LIB_NAME)
	$(CC) $(CFLAGS) -Imarshaller/demo/ -Isrc/ -Imarshaller/lib/ -o $@ $^

marshaller/gen/demo.tab.c: marshaller/demo/demo.h marshaller/marshaller-gen
	./marshaller/marshaller-gen -o $@ $<
	
marshaller/marshaller-gen:
	$(MAKE) -C marshaller/ marshaller-gen

marshaller-test: marshaller/gen/test.tab.c marshaller/test/test.c marshaller/lib/marshaller.c $(A_LIB_NAME)
	$(CC) -g -Imarshaller/test/ -Isrc/ -Imarshaller/lib/ -o $@ $^

marshaller/gen/test.tab.c: marshaller/test/test*.h marshaller/marshaller-gen
	./marshaller/marshaller-gen -o $@ marshaller/test/test*.h

clean:
	@echo "Cleaning up..."
	@rm -f obj/*.o
	@rm -f obj/*.d
	@rm -f test
	@rm -f $(BIN_NAME)
	@rm -f $(A_LIB_NAME)
	@rm -f $(SO_LIB_NAME)
	$(MAKE) -C marshaller/ clean
