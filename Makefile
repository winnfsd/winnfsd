PROGRAM := winnfsd.exe
SRCDIR  := src
BUILDDIR := build
SRCS := conv.cpp DatagramSocket.cpp FileTable.cpp FileTree.cpp MountProg.cpp NFS3Prog.cpp NFSProg.cpp PortmapProg.cpp RPCProg.cpp RPCServer.cpp ServerSocket.cpp Socket.cpp SocketStream.cpp winnfsd.cpp
OBJS := $(SRCS:%.cpp=$(BUILDDIR)/%.o)

ARCH ?= x86_64
RM  := rm -rf
CC=$(ARCH)-w64-mingw32-g++
CXX=$(ARCH)-w64-mingw32-g++

CPPFLAGS:=-g -O2 -std=c++11 -D_WIN32_WINNT=0x600 -I$(SRCDIR)
LDFLAGS:=-g -static-libgcc -static-libstdc++ -mconsole
LIBS:=-lws2_32 -lshlwapi

.PHONY: all clean

all: $(BUILDDIR)/$(PROGRAM)

$(BUILDDIR)/$(PROGRAM): $(OBJS)
	$(LINK.o) $^ $(LIBS) -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

$(BUILDDIR):
	mkdir -p $@

clean:
	$(RM) $(BUILDDIR)
