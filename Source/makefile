CXX	= g++
CXXFLAGS= -Wall -ansi -g
CC	= gcc
CCFLAGS= -g
INC	= -I/usr/lib/dlib -I/usr/lib/sqlite
LIBS	= -lpthread -ldl
OBJS	= main.o battlefield3.o bf3_socket.o helper.o http.o notifymyandroid.o mail.o vote.o translator.o SQLite.o extern_dlib.o extern_sqlite3.o

daspirum  : $(OBJS)
	$(CXX) $(INC) -o ../Release/$@ $(OBJS) $(LIBS)

%.o : src/%.cpp
	$(CXX) $(INC) -c $(CXXFLAGS) $<

%.o : src/%.c
	$(CC) $(INC) -c $(CCFLAGS) $<