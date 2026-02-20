#!/usr/bin/make -f

#-------------------------------------------------------------------------------
# 1. Setting up necessary variables for the build process
#-------------------------------------------------------------------------------

# The compilers that we will be using (we will only use g++)
CC := gcc
CXX := g++

# $(wildcard *.h) finds all file names with patterns (random string + ".h")
HDRS := $(wildcard *.h)		
SRCS := $(wildcard *.cpp)	
OBJS := $(SRCS:.cpp=.o)		# replaces .cpp extension to .o (e.g., main.cpp -> main.o)
				# and stores the names to OBJS

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
CFLAGS := -Wall -std=c++11 

# -pthread: use of posix threads (necessary to use std::thread or pthreads)
LFLAGS := -pthread 

# we are building two target binaries: server_app and client
TARGET := server_app client_app

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

debug: DFLAGS := -ggdb -DDEBUG
debug: $(TARGET)

# server_app: build the server binary to avoid conflict with the server directory
server_app: $(SVR_OBJS) $(CMN_OBJS)
	$(CXX) $(LFLAGS) -o $@ $^ 

# This rule defines how to build the server specific object files.
# To build object files, corresponding source code files (i.e., cpp and h files) are
# necessary. 
# Since we will have all the files the build command will be executed
#   g++ -Wall -std=c++0x -c ServerXXX1.cpp ServerXXX2.cpp ...

$(SVR_OBJS): $(SVR_SRCS) $(SVR_HDRS)
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(SVR_SRCS)



# client_app: build the client binary to match the target name
client_app: $(CLNT_OBJS) $(CMN_OBJS)
	$(CXX) $(LFLAGS) -o $@ $^ 

$(CLNT_OBJS): $(CLNT_SRCS) $(CLNT_HDRS)
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(CLNT_SRCS)
	

# This rule compiles the common source code into object files.
$(CMN_OBJS): $(CMN_SRCS) $(CMN_HDRS)
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(CMN_SRCS)

# This defines how you will clean up the compiled files.
# You can type "make clean" in the command line to delete all compiled files
# which include .o files and the compiled binary.

clean:
	rm -rf *.o $(TARGET)

# This indicates "clean" is not a target file to build but rather a
# special command.

.PHONY: clean debug

