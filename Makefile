CC       = gcc
CFLAGS   = -std=c99 -Wall -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -static -g
LD       = gcc
LDFLAGS  = 
AR       = ar
ARFLAGS  = rcs

BIN_NAME = demo
LIB_NAME = libcjson.a

OBJS     = obj/base.o obj/parse.o obj/query.o obj/stringify.o
DEPS     = $(OBJS:%.o=%.d)

all: $(BIN_NAME) $(LIB_NAME) test


$(BIN_NAME): obj/demo.o $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(LIB_NAME): CFLAGS += -fPIC
$(LIB_NAME): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

test: obj/test.o $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

-include $(DEPS)

obj/%.o: src/%.c obj
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

obj:
	@mkdir -p obj

clean:
	@echo "Cleaning up..."
	@rm -f obj/*.o
	@rm -f obj/*.d
	@rm -f test
	@rm -f $(BIN_NAME)
	@rm -f $(LIB_NAME)
