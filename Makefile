OBJDIR = obj
SRCDIR = src
INCSEARCHDIR = $(SRCDIR)/include
INCDIR = $(INCSEARCHDIR)/structures
BINDIR = bin
LIBDIR = lib
LIBOBJ = $(OBJDIR)/union_find.o $(OBJDIR)/suffix_tree.o
LIB = $(LIBDIR)/libstructures.a
TESTOBJ =$(OBJDIR)/tests.o
HEADERS = $(INCDIR)/suffix_tree.hpp $(INCDIR)/union_find.hpp $(INCDIR)/min_max_heap.hpp $(INCDIR)/immutable_list.hpp
CXX = g++
CPPFLAGS = -std=c++11 -m64 -g -I$(INCSEARCHDIR)


all:
	make $(BINDIR)/test

.PHONY: clean .pre_build
clean:
	find $(BINDIR) $(OBJDIR) $(LIBDIR) -type f -delete

$(BINDIR)/test: $(TESTOBJ) $(LIB) 
	$(CXX) $(CPPFLAGS) -o $(BINDIR)/test $(TESTOBJ) $(LIB)

$(OBJDIR)/suffix_tree.o: $(SRCDIR)/suffix_tree.cpp $(INCDIR)/suffix_tree.hpp
	$(CXX) $(CPPFLAGS) -c $(SRCDIR)/suffix_tree.cpp -o $(OBJDIR)/suffix_tree.o 

$(OBJDIR)/union_find.o: $(SRCDIR)/union_find.cpp $(INCDIR)/union_find.hpp
	$(CXX) $(CPPFLAGS) -c $(SRCDIR)/union_find.cpp -o $(OBJDIR)/union_find.o 

# MinMaxHeap is header-only

$(OBJDIR)/tests.o: $(SRCDIR)/tests.cpp $(HEADERS)
	$(CXX) $(CPPFLAGS) -c $(SRCDIR)/tests.cpp -o $(OBJDIR)/tests.o 
	
test: $(BINDIR)/test
	./bin/test

$(LIB): $(LIBOBJ)
	rm -f $@
	ar rs $@ $(LIBOBJ)

.pre-build:
	if [ ! -d $(BINDIR) ]; then mkdir -p $(BINDIR); fi
	if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi
	if [ ! -d $(LIBDIR) ]; then mkdir -p $(LIBDIR); fi

# run .pre-build before we make anything at all.
-include .pre-build

