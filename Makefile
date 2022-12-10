
#
#	Makefile for qdb_test project
#

DEBUG = -g
CC = qcc
LD = qcc
PLAT = x86_64

TARGET = -Vgcc_ntox86_64
#TARGET = -Vgcc_ntox86
#TARGET = -Vgcc_ntoarmv7le
#TARGET = -Vgcc_ntoaarch64le


CFLAGS += $(DEBUG) $(TARGET) -Wall 
LDFLAGS+= $(DEBUG) $(TARGET) -lqdb -lsqlite3 -lsocket
BINS = qdb_ews qdb_no_ews remote_test
LIBS = includes\$(PLAT)\libqdb.so includes\$(PLAT)\libsqlite3.so includes\$(PLAT)\libicui18n.so includes\$(PLAT)\libicudata.so
OBJS = db_access.o

all: $(OBJS) $(BINS) $(LIBS)

remote_test: remote_test.c db_access.o

qdb_ews: qdb_ews.c db_access.o

db_access.o: db_access.c db_access.h
	$(CC) $(CFLAGS) -c db_access.c
	
clean:
	rm -f *.o $(BINS) $(OBJS)
#	cd solutions; make clean

#example.o: example.c
#qdb_test.o: qdb_test.c