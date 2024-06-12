SRC=src
CC=gcc
CFLAGS=-I $(SRC)

ODIR=obj
TDIR=test

_DEPS = util.h request.h response.h
DEPS = $(patsubst %,$(SRC)/%,$(_DEPS))

_OBJS = server.o util.o request.o response.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

$(ODIR)/%.o: $(SRC)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJS)
	$(CC) -o Served $^ $(CFLAGS)

clean:
	rm -rf $(ODIR)/*.o
