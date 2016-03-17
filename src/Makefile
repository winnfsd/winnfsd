PROGRAM = winnfsd.exe
SRCS = DatagramSocket.cpp FileTable.cpp FileTree.cpp MountProg.cpp NFS2Prog.cpp NFS3Prog.cpp NFSProg.cpp PortmapProg.cpp RPCProg.cpp RPCServer.cpp ServerSocket.cpp Socket.cpp SocketStream.cpp winnfsd.cpp
OBJS = $(subst .cpp,.o,$(SRCS))

RM  := rm
CC  := x86_64-w64-mingw32-g++
CXX := x86_64-w64-mingw32-g++
#CC  := i686-w64-mingw32-g++
#CXX := i686-w64-mingw32-g++

CPPFLAGS = -std=c++11 -D_WIN32_WINNT=0x600
LDFLAGS = -static-libgcc -static-libstdc++ -mconsole
LIBS = -lws2_32

$(PROGRAM): $(OBJS)
	$(LINK.o) $(LDFLAGS)  $^ $(LIBS) -o $@

.PHONY: clean

clean:
	$(RM) $(OBJS) $(PROGRAM)
