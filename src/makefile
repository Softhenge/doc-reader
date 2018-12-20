SOURCES = main.cpp \
          api.cpp \
 		  helper.cpp \
		  scanner.cpp

OBJDIR = ../obj
OBJECTS = $(SOURCES:.cpp=.o)
MOVED_OBJECTS = $(adprefix ./$(OBJDIR)/,$(OBJECTS))
CC = g++

CPPFLAGS   = -O0 -std=c++14 -Wall -Werror -g $(shell pkg-config --cflags tesseract) -c
#CPPFLAGS = -std=c++14 -c

.PHONY: clean all
all: $(OBJECTS) $(SOURCES)
	mkdir -p $(OBJDIR)
	mv *.o $(OBJDIR)

.cpp.o:
	$(CC) $(CPPFLAGS) $< -o $@

clean:
	rm -rf $(OBJDIR)
