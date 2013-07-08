TARGET=$(shell basename `pwd`)
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:%.cpp=%.o)

CFLAGS=-lpthread -lcurses
LDFLAGS=

all: $(TARGET)

$(OBJECTS): $(SOURCES)

$(TARGET): $(OBJECTS)
	$(CXX) -o $(TARGET) $(LDFLAGS) $(OBJECTS) $(CFLAGS)
clean:
	$(RM) $(OBJECTS) $(TARGET)

.PHONY: all clean