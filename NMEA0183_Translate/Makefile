CC = g++
CFLAGS = -std=c++11
LDFLAGS = -static-libstdc++ -lws2_32 -static-libgcc

SRCDIR = src
OBJDIR = obj
BINDIR = bin
INCDIR = include

# List of source files for Service and Client
SRCS_Service := $(SRCDIR)/Service.cpp
SRCS_Client := $(SRCDIR)/Client.cpp

# Get a list of all .cpp files excluding Service.cpp and Client.cpp
ALL_CPP := $(filter-out $(SRCS_Service) $(SRCS_Client), $(wildcard $(SRCDIR)/*.cpp))

# Object files for Service and Client
OBJS_Service := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%_Service.o,$(SRCS_Service))
OBJS_Client := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%_Client.o,$(SRCS_Client))

# Object files for all other sources
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(ALL_CPP))

# Executables for Service and Client
EXECUTABLE_Service := $(BINDIR)/service.exe
EXECUTABLE_Client := $(BINDIR)/client.exe

all: $(EXECUTABLE_Service) $(EXECUTABLE_Client)

$(EXECUTABLE_Service): $(OBJS_Service) $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) 

$(EXECUTABLE_Client): $(OBJS_Client) $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) 

$(OBJDIR)/%_Service.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -DSERVICE_BUILD -I$(INCDIR)

$(OBJDIR)/%_Client.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -DCLIENT_BUILD -I$(INCDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	del /Q "$(OBJDIR)\*.o" "$(BINDIR)\client.exe" "$(BINDIR)\service.exe"

.PHONY: all clean
