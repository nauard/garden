OBJECTS = main.o individu.o board.o cellule.o utilities.o textInterface.o

CPP = g++
CFLAGS = -g -Wall
LIBS = -lstdc++

TARGET = client world


all: $(TARGET)

world: $(OBJECTS) libcsock.a
	$(CPP) $(OBJECTS) -lX11 -lpthread -L. -lcsock -o $@

client: client.c libcsock.a
	g++ -g -Wall -o client client.c -L. -lcsock

%.o: %.cpp spec.h
	$(CPP) $(CFLAGS) -c $< -o $@

libcsock.a: csock.h csock.c
	g++ -g -Wall -c -o libcsock.a csock.c

clean:
	rm -f $(TARGET) $(OBJECTS) cmd_input cmd_output libcsock.a *.log

cleanall: clean
	rm TAGS *~

tags:
	if [ -f TAGS ]; then rm TAGS;fi
	find . -name '*.cpp' | xargs etags -a
	find . -name '*.h'   | xargs etags -a
