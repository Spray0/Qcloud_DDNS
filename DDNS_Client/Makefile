TARGET = DDNS_Client
CC = g++
RM = rm -rf
FLAGS = -pthread -std=c++11 $(INCLUDES)

$(TARGET)	:  SRC/main.cpp
	@$(CC)  -o $(TARGET)  SRC/main.cpp $(FLAGS)

clean : 
	@$(RM)  $(TARGET) DDNS.log