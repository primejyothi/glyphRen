SOURCES = glyphRen.cc fontClass.cc fontClass.hpp jlog.cc jlog.hpp
OBJS = glyphRen.o fontClass.o jlog.cc
EXEC = glyphRen
CC = g++

CCFLAGS = -g  -Wall

.PHONY : all clean

all : $(EXEC)

glyphRen.o : glyphRen.cc fontClass.hpp jlog.hpp
fontClass.o : fontClass.cc fontClass.hpp jlog.hpp
jlog.o : jlog.hpp

$(EXEC) : $(OBJS)
	$(CC) $(CCFLAGS) $(LPATH) -o $@ $^  $(LIBFLAGS)

.cc.o :
	$(CC) -c $(CCFLAGS) -o $@ $< 
.o.hpp :
	$(CC) -c $(CCFLAGS) -o $@ $< 
docs : $(SOURCES) docs.cfg
	doxygen docs.cfg
clean :
	rm -f $(EXEC) *.o
