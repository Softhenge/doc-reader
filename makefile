CC = g++
SUBDIRS    = src
OBJDIR     = obj
BINDIR     = bin
DOXY       = ./docs/doxy
OBJECTS    = $(SOURCES:.cpp:=.o)
EXECUTABLE = doc-reader 
CPPFLAGS   = -O3 -std=c++14 -Wall -Werror -g $(shell pkg-config --cflags tesseract)
LDFLAGS    =  $(shell pkg-config --libs opencv tesseract) -lpthread -lstdc++fs

MOVED_OBJECTS =  $(addprefix $(OBJDIR)/, $(OBJECTS))

.PHONY : all clean doxygen

all: $(OBJECTS)
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)
	mkdir -p $(DOXY)
	$(foreach subdir, $(SUBDIRS), cd $(subdir); make all; cd ../;)
	$(CC) $(OBJDIR)/* $(LDFLAGS) -o $(EXECUTABLE)
	mv $(EXECUTABLE) $(BINDIR)

doxygen:
	@doxygen	./docs/Doxyfile

clean:
	rm -rf $(BINDIR) $(OBJDIR) $(DOXY)
	$(foreach subdir,$(SUBDIRS), cd $(subdir); make clean; cd ../;)
