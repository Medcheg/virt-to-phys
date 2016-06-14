#/**
# * \file Makefile
# * \brief makefile for ta generator tool
# * \author Pavel Marusyk, p.marusik@samsung.com
# * \version 1.0
# * \date Created September 11, 2012 15:00 PM
# * \par In Samsung Ukraine R&D Center (SURC) under a contract between
# * \par LLC "Samsung Electronics Ukraine Company" (Kiev, Ukraine) and "Samsung Elecrtronics Co", Ltd (Seoul, Republic of Korea)
# * \par Copyright: (c) Samsung Electronics Co, Ltd 2012. All rights reserved.
#**/


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
