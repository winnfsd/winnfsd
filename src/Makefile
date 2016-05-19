PROGRAM := winnfsd.exe
SRCS := conv.cpp DatagramSocket.cpp FileTable.cpp FileTree.cpp MountProg.cpp NFS3Prog.cpp NFSProg.cpp PortmapProg.cpp RPCProg.cpp RPCServer.cpp ServerSocket.cpp Socket.cpp SocketStream.cpp winnfsd.cpp
OBJS = $(subst .cpp,.o,$(SRCS))

ARCH ?= x86_64
RM  := rm
CC=$(ARCH)-w64-mingw32-g++
CXX=$(ARCH)-w64-mingw32-g++

CPPFLAGS:=-g -O2 -std=c++11 -D_WIN32_WINNT=0x600
LDFLAGS:=-g -static-libgcc -static-libstdc++ -mconsole
LIBS:=-lws2_32

$(PROGRAM): $(OBJS)
	$(LINK.o) $^ $(LIBS) -o $@

.PHONY: clean

clean:
	$(RM) $(OBJS) $(PROGRAM)
