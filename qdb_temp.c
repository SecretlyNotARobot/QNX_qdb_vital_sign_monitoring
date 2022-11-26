#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>
#include <sys/netmgr.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <errno.h>
#include <string.h>
#include <qdb/qdb.h>
#include <sqlite3.h>
#include <time.h>

#include "server.h" // defines messages between client and server

/** SQLite3 API Documentation:
 *
 * https://www.sqlite.org/cintro.html
 * https://www.sqlite.org/rescode.html#ok
 *
 */

//channel id
int coid;

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	char rmsg[MAX_STRING_LEN];
	struct get_vital_msg vmsg;
	float val;
	long msg_send_id;

	//Get column value
	val = atof(argv[0]);
	//print it
	//printf("%s = %2.2f\n", azColName[0], val);
	//send it to early warning score server
	vmsg.type = TEMPERATURE;
	vmsg.vital_data = val;
	msg_send_id = MsgSend(coid, &vmsg, sizeof(vmsg), rmsg, sizeof(rmsg));
	if(msg_send_id == -1){
		perror("MsgSend()");
		exit(-1);
	}

    return 0;
}

int main(int argc, char **argv) {
	//time measurement
	clock_t start, end;
	double cpu_time_used;

	//db variables
	int rc; //return code
	sqlite3 *handle;
	char *select = "select temperature from vitalperiodic limit 10";
	char *errmsg;

	//Connect to channel
	coid = name_open("temp", 0);

	sqlite3_open("/tmp/eicu_v2_0_1_copy.sqlite3", &handle);

	//Wrapper function that does sqlite3_prepare(), sqlite3_step(), sqlite3_column(),
	//	and sqlite3_finalize() for one or more SQL statements
	start = clock();
	rc = sqlite3_exec(handle, select, callback, NULL, &errmsg);
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	//Error Checking
	if (rc == SQLITE_ERROR) {
	      fprintf(stderr, "Error executing SELECT statement: %s\n", errmsg);
	      return EXIT_FAILURE;
	   }
	else {
		printf("Successfully retrieved from database! in %2.8f s\n", cpu_time_used);
	}
	return 0;
}
