all: hello

hello: main.o gpio.o pwm.o
	g++ main.o gpio.o pwm.o -o hello -lpthread -lcurses

main.o: main.cpp
	g++ -c main.cpp

gpio.o: gpio.cpp
	g++ -c gpio.cpp

pwm.o: pwm.cpp
	g++ -c pwm.cpp

clean:
	rm -rf *.o hello