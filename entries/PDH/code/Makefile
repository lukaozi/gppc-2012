OBJS = main.o Entry.o ScenarioLoader.o Timer.o
CC = g++
CFLAGS = -O3 -c -std=c++0x
LFLAGS = -O3 -o

bdragon : $(OBJS)
	$(CC) $(LFLAGS) bdragon $(OBJS) -pthread
	\rm *.o

main.o : main.cpp Entry.h ScenarioLoader.h Timer.h
	$(CC) $(CFLAGS) main.cpp 

Entry.o : Entry.cpp Entry.h
	$(CC) $(CFLAGS) Entry.cpp

ScenarioLoader.o : ScenarioLoader.cpp ScenarioLoader.h
	$(CC) $(CFLAGS) ScenarioLoader.cpp

Timer.o : Timer.cpp Timer.h
	$(CC) $(CFLAGS) Timer.cpp

clean:
	\rm *.o bdragon
