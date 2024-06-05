SRC=src
CC=gcc
CFLAGS=-I $(SRC)

ODIR=obj
TDIR=test

_DEPS = parser.h util.h
DEPS = $(patsubst %,$(SRC)/%,$(_DEPS))

_OBJS = server.o parser.o util.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

$(ODIR)/%.o: $(SRC)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJS)
	$(CC) -o Served $^ $(CFLAGS)

clean:
	rm -rf $(ODIR)/*.o
