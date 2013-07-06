all: hello

hello: main.o gpio.o pwm.o spi.o msp_pwm.o
	g++ main.o gpio.o pwm.o spi.o msp_pwm.o -o hello -lpthread -lcurses

main.o: main.cpp
	g++ -c main.cpp

gpio.o: gpio.cpp
	g++ -c gpio.cpp

pwm.o: pwm.cpp
	g++ -c pwm.cpp

spi.o: spi.cpp
	g++ -c spi.cpp

msp_pwm.o: msp_pwm.cpp
	g++ -c msp_pwm.cpp

clean:
	rm -rf *.o hello