

TARGET = demo
CC = g++
RM = rm -rf
INCLUDES = -IQcloud_DNS_API/
FLAGS = -pthread -lcurl -lcrypto -std=c++11 $(INCLUDES)
OBJECTS = Qcloud_DNS_API/QCloud_DNS_API.o SRC/main.o 


all: obj out
out	:	$(OBJECTS)
	@$(CC)  -o $(TARGET)  $(OBJECTS) $(FLAGS)
obj :
	@$(CC) -c -o SRC/main.o SRC/main.cpp
	@$(CC) -c -o Qcloud_DNS_API/QCloud_DNS_API.o Qcloud_DNS_API/QCloud_DNS_API.cpp

clean : 
	@$(RM) $(OBJECTS) $(TARGET) DDNS.log