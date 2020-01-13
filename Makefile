CC            = g++
CFLAGS        = -O3 -Wall
LDFLAGS       =
LIBS          = -lm
OBJS          = token_reader.o table.o compiler.o stackmachine.o
PROGRAM       = a

all:            $(PROGRAM)

$(PROGRAM):     $(OBJS)
				$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $(PROGRAM)
