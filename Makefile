
NAME = ./virt_to_phys
IDIR = ./
CC = gcc
CFLAGS = -I$(IDIR) -Wall
ODIR = .

_DEPS = *.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

$(NAME): *.c

	@gcc -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	@rm -f *.o *~ core $(INCDIR)/*~ $(NAME)
