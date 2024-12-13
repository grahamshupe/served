SRC=src
CC=gcc
CFLAGS=-I $(SRC) -g
TFLAGS=-Wall -g -lcunit

ODIR=obj
TDIR=test

TARGET=Served
TESTTARGET=test_served

_DEPS = util.h request.h response.h zf_log.h handler.h
DEPS = $(patsubst %,$(SRC)/%,$(_DEPS))

_OBJS = server.o util.o request.o response.o zf_log.o handler.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

_TESTS = test.o request_test.o response_test.o
TESTS = $(patsubst %,$(TDIR)/%,$(_TESTS))

$(ODIR)/%.o: $(SRC)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(TDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(TFLAGS)

server: $(OBJS)
	$(CC) -o $(TARGET) $^ $(CFLAGS)

clean:
	rm -rf $(ODIR)/*.o $(TDIR)/*.o

test: $(TESTS) $(filter-out $(ODIR)/server.o, $(OBJS))
	$(CC) -o $(TESTTARGET) $^ $(TFLAGS)