TARGET=$(shell basename `pwd`)
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:%.cpp=%.o)

CFLAGS=-lpthread -lcurses -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lopencv_ml -lopencv_video -lopencv_calib3d -lopencv_contrib -lopencv_legacy -lopencv_flann
LDFLAGS=

all: $(TARGET)

$(OBJECTS): $(SOURCES)

$(TARGET): $(OBJECTS)
	$(CXX) -o $(TARGET) $(LDFLAGS) $(OBJECTS) $(CFLAGS) 
clean:
	$(RM) $(OBJECTS) $(TARGET)

.PHONY: all clean