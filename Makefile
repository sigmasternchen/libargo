CC       = gcc
CFLAGS   = -std=c99 -Wall -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -static -g
LD       = gcc
LDFLAGS  = 
AR       = ar
ARFLAGS  = rcs

BIN_NAME = demo
A_LIB_NAME = libcjson.a
SO_LIB_NAME = libcjson.so

OBJS     = obj/base.o obj/parse.o obj/query.o obj/stringify.o
DEPS     = $(OBJS:%.o=%.d)

all: $(A_LIB_NAME) $(SO_LIB_NAME) $(BIN_NAME) test


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

clean:
	@echo "Cleaning up..."
	@rm -f obj/*.o
	@rm -f obj/*.d
	@rm -f test
	@rm -f $(BIN_NAME)
	@rm -f $(A_LIB_NAME)
	@rm -f $(SO_LIB_NAME)
