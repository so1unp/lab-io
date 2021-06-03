CC=gcc
BIN=./bin
CFLAGS=-Wall -g

PROG=write_bytes

LIST=$(addprefix $(BIN)/, $(PROG))

.PHONY: all
all: $(PROG)

%: %.c $(LIBS)
	$(CC) -o $(BIN)/$@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(LIST)
