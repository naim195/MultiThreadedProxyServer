# Use the g++ compiler for C++
CC=g++
# Enable C++17 standard for features like std::thread, and link the pthread library
CFLAGS= -g -Wall -std=c++17 -pthread

# Define the final executable name
TARGET=proxy

# Define all the object files that will be created
OBJECTS=main.o ProxyServer.o CacheManager.o HTTPRequest.o

# The default 'all' rule, which builds the final executable
all: $(TARGET)

# Rule to link all the object files into the final executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Rule to compile main.cpp into main.o
main.o: main.cpp ProxyServer.hpp
	$(CC) $(CFLAGS) -c main.cpp

# Rule to compile ProxyServer.cpp into ProxyServer.o
ProxyServer.o: ProxyServer.cpp ProxyServer.hpp CacheManager.hpp HTTPRequest.hpp
	$(CC) $(CFLAGS) -c ProxyServer.cpp

# Rule to compile CacheManager.cpp into CacheManager.o
CacheManager.o: CacheManager.cpp CacheManager.hpp
	$(CC) $(CFLAGS) -c CacheManager.cpp

# Rule to compile HTTPRequest.cpp into HTTPRequest.o
HTTPRequest.o: HTTPRequest.cpp HTTPRequest.hpp
	$(CC) $(CFLAGS) -c HTTPRequest.cpp

# The 'clean' rule to remove all compiled files
clean:
	rm -f $(TARGET) $(OBJECTS)