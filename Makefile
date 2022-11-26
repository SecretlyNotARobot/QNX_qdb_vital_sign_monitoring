
#
#	Makefile for qdb_test project
#

DEBUG = -g
CC = qcc
LD = qcc


TARGET = -Vgcc_ntox86_64
#TARGET = -Vgcc_ntox86
#TARGET = -Vgcc_ntoarmv7le
#TARGET = -Vgcc_ntoaarch64le


CFLAGS += $(DEBUG) $(TARGET) -Wall -lqdb -lsqlite3
LDFLAGS+= $(DEBUG) $(TARGET)
BINS = qdb_test qdb_temp qdb_respiration qdb_heartrate qdb_early_warning_score qdb_no_ews
all: $(BINS)

all:	qdb_test qdb_temp qdb_respiration qdb_heartrate qdb_early_warning_score qdb_no_ews

clean:
	rm -f *.o $(BINS)
#	cd solutions; make clean

#example.o: example.c
#qdb_test.o: qdb_test.c