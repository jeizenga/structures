CXX = g++
CPPFLAGS = -std=c++11 -m64
OBJ = $(OBJDIR)/min_max_heap.o $(OBJDIR)/union_find.o $(OBJDIR)/suffix_tree.o $(OBJDIR)/tests.o
OBJDIR = obj
SRCDIR = src
BINDIR = bin

all:
	make $(BINDIR)/test

.PHONY: clean .pre_build
clean:
	@if [ $$(find bin -type f | wc -l) -gt 0 ]; \
	then { \
		echo "The following will be deleted:"; \
		echo "------------------------------"; \
		find $(BINDIR) $(OBJDIR) -type f; \
		echo "------------------------------"; \
		read -p "Continue (y/n)? " -n 1 -r CONTINUE; \
		echo; \
	}; \
	else echo "No files to delete."; \
	fi; \
	\
	if [[ $$CONTINUE =~ ^[Yy]$$ ]]; \
	then find $(BINDIR) $(OBJDIR) -type f -delete; \
	else echo "Aborted"; \
	fi;

$(BINDIR)/test: $(OBJ)
	$(CXX) $(CPPFLAGS) -o $(BINDIR)/test $(OBJ)

$(OBJDIR)/suffix_tree.o: $(SRCDIR)/suffix_tree.hpp
	$(CXX) $(CPPFLAGS) -c $(SRCDIR)/suffix_tree.hpp -o $(OBJDIR)/suffix_tree.o 

$(OBJDIR)/union_find.o: $(SRCDIR)/union_find.hpp
	$(CXX) $(CPPFLAGS) -c $(SRCDIR)/union_find.hpp -o $(OBJDIR)/union_find.o 

$(OBJDIR)/min_max_heap.o: $(SRCDIR)/min_max_heap.hpp
	$(CXX) $(CPPFLAGS) -c $(SRCDIR)/min_max_heap.hpp -o $(OBJDIR)/min_max_heap.o

$(OBJDIR)/tests.o: $(SRCDIR)/tests.cpp
	$(CXX) $(CPPFLAGS) -c $(SRCDIR)/tests.cpp -o $(OBJDIR)/tests.o 
	
test: $(BINDIR)/test
	./bin/test

.pre-build:
	if [ ! -d $(BINDIR) ]; then mkdir -p $(BINDIR); fi
	if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi

# run .pre-build before we make anything at all.
-include .pre-build

