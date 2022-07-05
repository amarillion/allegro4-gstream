## Makefile --- gstream
#
# This makefile is from DRS by Michal Bukin,
# though heavily modified.
# 
# Ole Laursen


# Either copy makedoc from allegro/docs to this directory
# or change the MAKEDOC line below to your allegro/docs directory

ALLEGROLIB = -lalleg
EXE = .exe
CXX = c++

CC = gcc
WFLAGS = -Wall
OFLAGS = -mcpu=i486 -O3 -ffast-math -fomit-frame-pointer
# OFLAGS = -g
CPPFLAGS = 
CFLAGS = $(WFLAGS) $(OFLAGS)
CXXFLAGS = $(WFLAGS) $(OFLAGS)
INCLUDES = -I$(MINGDIR)/include/c++
DEFS = 
LDFLAGS =

LIBS = $(ALLEGROLIB)

AR = ar
RM = del /q

prefix = $(MINGDIR)
bindir = $(prefix)\bin
libdir = $(prefix)\lib
includedir = $(prefix)\include

COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)
LINK = $(CC) $(CFLAGS) $(LDFLAGS) -o $@

CXX_COMPILE = $(CXX) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CXXFLAGS)
CXX_LINK = $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@

LDADD = libgstrm.a

exe_TARGETS = test$(EXE)
test_exe_OBJECTS = test.o
test_exe_DEPENDENCIES = libgstrm.a
test_exe_LDADD = $(LDADD)
test_exe_LDFLAGS = 

lib_TARGETS = libgstrm.a
libgstrm_a_OBJECTS = gsintfac.o gsvirtul.o gsmanip.o gswrirea.o gsfunc.o
libgstrm_a_DEPENDENCIES = 
libgstrm_a_LIBADD = 

targets = $(lib_TARGETS) $(exe_TARGETS)

.PHONY: all
all: $(targets)

.PHONY: install
install: $(lib_TARGETS)
	copy /y libgstrm.a $(libdir)
	copy /y gstream.h $(includedir)
	copy /y gstream $(includedir)
	copy /y gmanip.h $(includedir)
	copy /y gmanip $(includedir)


%.o: %.c
	$(COMPILE) -c $<

%.o: %.cc gstream.h
	$(CXX_COMPILE) -c $<

%.o: %.cpp
	$(CXX_COMPILE) -c $<

test$(EXE): $(test_exe_OBJECTS) $(test_exe_DEPENDENCIES)
	$(CXX_LINK) $(test_exe_LDFLAGS) $(test_exe_OBJECTS) $(test_exe_LDADD) $(LIBS)

libgstrm.a: $(libgstrm_a_OBJECTS) $(libgstrm_a_DEPENDENCIES)
	$(AR) rs $@ $(libgstrm_a_OBJECTS) $(libgstrm_a_LIBADD)

gstream.txi: docs

TAGS: *.cc *.h
	etags *.cc "--language=c++" *.h

.PHONY: tags
tags: TAGS

.PHONY: clean
clean:
	-$(RM) *.o

.PHONY: distclean
distclean: clean
	-$(RM) $(targets)
	-$(RM) *.ht* gstream.txi gstream.rtf gstream.txt