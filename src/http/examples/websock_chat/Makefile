CXX=g++
CXXFLAGS=-g -ggdb -fsanitize=address -Wall -std=c++17 -Itemplates -I../../htcc -I../.. -I ../MiniJson/Source/include
HTCC=../../htcc/htcc
LIBS=-fsanitize=address -std=c++17 -pthread

all: websock_chat_demo

include ../../http.mk

templates/%.html.hpp: templates/%.html
	$(HTCC) $< $@

websock_chat_demo: build/chat_socket.o build/demo.o build/user_control.o build/http.o build/websock.o
	$(CXX) $(LIBS) \
		build/http.o \
		build/websock.o \
		build/chat_socket.o \
		build/demo.o \
		build/user_control.o \
		../MiniJson/Source/libJson.a -o websock_chat_demo

build/chat_socket.o: src/chat_socket.cpp src/websock_chat.h
	$(CXX) $(CXXFLAGS) -c src/chat_socket.cpp -o build/chat_socket.o

build/demo.o: src/demo.cpp src/websock_chat.h templates/home.html.hpp
	$(CXX) $(CXXFLAGS) -c src/demo.cpp -o build/demo.o

build/user_control.o: src/user_control.cpp src/websock_chat.h
	$(CXX) $(CXXFLAGS) -c src/user_control.cpp -o build/user_control.o

clean:
	rm -f build/*.o websock_chat_demo
