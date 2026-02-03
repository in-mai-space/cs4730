#!/usr/bin/make -f

#-------------------------------------------------------------------------------
# 1. Setting up necessary variables for the build process
#-------------------------------------------------------------------------------

# The compilers that we will be using (we will only use g++)
CC := gcc
CXX := g++

# Source and header files for each component
SVR_HDRS := $(wildcard server/include/*.h)
SVR_SRCS := $(wildcard server/src/*.cpp)
SVR_OBJS := $(notdir $(SVR_SRCS:.cpp=.o))

CLNT_HDRS := $(wildcard client/include/*.h)
CLNT_SRCS := $(wildcard client/src/*.cpp)
CLNT_OBJS := $(notdir $(CLNT_SRCS:.cpp=.o))

CMN_HDRS := $(wildcard common/include/*.h)
CMN_SRCS := $(wildcard common/src/*.cpp)
CMN_OBJS := $(notdir $(CMN_SRCS:.cpp=.o))


# -Wall prints: all warnings
# -std=c++11: use of C++11  
# -I flags: include directories for header files
CFLAGS := -Wall -std=c++11 -Iclient/include -Iserver/include -Icommon/include 

# -pthread: use of posix threads (necessary to use std::thread or pthreads)
LFLAGS := -pthread 

# we are building two target binaries: server_binary and client_binary
TARGET := server_binary client_binary

#-------------------------------------------------------------------------------
# 2. What to build and how to build them
#-------------------------------------------------------------------------------

# The build rules are defined as follows:
# target : prerequisite files
#	How to build the target
# 1. Make program will first check whether the target exists in the folder.
# 2. If not it will check for prerequisite files.
# 3. If the prerequisite files all exist in the folder the program will execute
#    the next line to build the target.
# 4. If any of the prerequisite file does not exist, the program will look for 
#    rules to create the prerequisite file: it will look for rules where the
#    prerequisite file is the target.


# "all:" is the starting point of the build process unless specified otherwise.
# 1. The make program will look for targets (i.e., server and client) to build
# 2. If the target is not found in the folder it will look for
#    rules to build the target (it will look for "server: xxx" and "client: xxx"
#    which should define the rules to build the server and the client)

all: $(TARGET)

# "debug:" allows the g++ compiler to embed debugging information that can be
# used for gdb debugger. To use this mode type "make debug." This command will 
# set the necessary compilation flag. Other build sequence is the same as
# "all" above that is triggered by the default "make" command.
debug: DFLAGS := -ggdb -DDEBUG
debug: $(TARGET)

# "server:" defines rules to build the server.
# 1. To build the server binary we need compiled object files for the server.
#    We defined them above and the necessary object files are:
#    - $(SVR_OBJS) that includes all server specific object files and
#    - $(CMN_OBJS) that include all common object files that are used for both
#      server and client programs.
# 2. The next line $(CXX) $(LFALGS) -o $@ $^ defines how to create server
#    binary. This line translates to 
#
#      g++ -pthread -o server ServerXXX1.o ServerXXX2.o ... Common1.o ...
#
#    $@ is a macro for the string behind the colon (i.e., server).
#    $^ is a macro for all the string after the colon (i.e., $(SVR_OBJS) $(CMN_OBJS))
#
# 3. When you start compiling, however, you will only have .cpp and .h files
#    in the folder the make program won't be able to execute the g++ command and
#    it will look for another rules to create the object (.o) files

server_binary: $(SVR_OBJS) $(CMN_OBJS)
	$(CXX) $(LFLAGS) -o $@ $^ 

# This rule defines how to build the server specific object files.
# To build object files, corresponding source code files (i.e., cpp and h files) are
# necessary. 
# Since we will have all the files the build command will be executed
#   g++ -Wall -std=c++0x -c ServerXXX1.cpp ServerXXX2.cpp ...

$(SVR_OBJS): $(SVR_SRCS) $(SVR_HDRS) $(CMN_HDRS)
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(SVR_SRCS)


# Same applies to the client program.
client_binary: $(CLNT_OBJS) $(CMN_OBJS)
	$(CXX) $(LFLAGS) -o $@ $^ 

$(CLNT_OBJS): $(CLNT_SRCS) $(CLNT_HDRS) $(CMN_HDRS)
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(CLNT_SRCS)
	

# This rule compiles the common source code into object files.
$(CMN_OBJS): $(CMN_SRCS) $(CMN_HDRS)
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(CMN_SRCS)

# This defines how you will clean up the compiled files.
# You can type "make clean" in the command line to delete all compiled files
# which include .o files and the compiled binary.

clean:
	rm -f *.o server_binary client_binary

# ".PHONY:" indicates these are not target files to build but 
# rather special commands.

.PHONY: clean debug run-server run-client format lint help

#-------------------------------------------------------------------------------
# 3. Convenient commands for development
#-------------------------------------------------------------------------------

# Start server with default settings (port 8080, 2 engineers)
run-server: server_binary
	@echo "Starting server on port 8080 with 2 expert engineers..."
	./server_binary 8080 2

# Start client with default settings (3 customers, 5 orders each, robot type 0)
run-client: client_binary
	@echo "Starting client with 3 customers, 5 orders each, robot type 0..."
	./client_binary 127.0.0.1 8080 3 5 0

# Start client with custom settings (usage: make run-client-custom IP=127.0.0.1 PORT=8080 CUSTOMERS=3 ORDERS=5 ROBOT=0)
run-client-custom: client_binary
	./client_binary $(IP) $(PORT) $(CUSTOMERS) $(ORDERS) $(ROBOT)

# Format code using clang-format (if available)
format:
	@echo "Formatting C++ code..."
	@if command -v clang-format >/dev/null 2>&1; then \
		find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i -style="{BasedOnStyle: Google, IndentWidth: 4, TabWidth: 4}"; \
		echo "Code formatted successfully."; \
	else \
		echo "clang-format not found. Install with: brew install clang-format"; \
	fi

# Lint code using clang-tidy (if available)
lint:
	@echo "Linting C++ code..."
	@if command -v clang-tidy >/dev/null 2>&1; then \
		find . -name "*.cpp" | xargs clang-tidy -checks="-*,readability-*,performance-*,modernize-*" --; \
		echo "Code linted successfully."; \
	else \
		echo "clang-tidy not found. Install with: brew install llvm"; \
	fi

# Debug server with lldb
debug-server: server_binary
	@echo "Starting server in debugger..."
	lldb -o "run 8080 2" ./server_binary

# Debug client with lldb
debug-client: client_binary
	@echo "Starting client in debugger..."
	lldb -o "run 127.0.0.1 8080 3 5 0" ./client_binary