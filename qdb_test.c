#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <string.h>
#include <qdb/qdb.h>
#include <sqlite3.h>

/** SQLite3 API Documentation:
 *
 * https://www.sqlite.org/cintro.html
 * https://www.sqlite.org/rescode.html#ok
 *
 */

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   printf("num args = %d", argc);
   printf("\n");
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   return 0;
}


int main(int argc, char **argv) {

	int rc; //return code
	sqlite3 *handle;
	char *select = "select * from vitalperiodic limit 1";
	char *errmsg;

	sqlite3_open("/tmp/eicu_v2_0_1_copy.sqlite3", &handle);


	//Wrapper function that does sqlite3_prepare(), sqlite3_step(), sqlite3_column(),
	//	and sqlite3_finalize() for one or more SQL statements

	rc = sqlite3_exec(handle, select, callback, NULL, &errmsg);

	//Error Checking
	if (rc == SQLITE_ERROR) {
	      fprintf(stderr, "Error executing SELECT statement: %s\n", errmsg);
	      return EXIT_FAILURE;
	   }
	else {
		printf("Successfully retrieved from database!\n");
	}
	return 0;
}









//	int i, j, ret, rows;
//	char *age_col = "SELECT age FROM patient";
//    qdb_result_t *result;
//    qdb_hdl_t *dbhandle;  // The QDB database handle
//    dbhandle = qdb_connect("/tmp/db/eicu_v2_0_1_copy.sqlite3",0);
//    if (dbhandle == NULL) {
//        printf("Connect failed\n");
//        exit(-10);
//    }
//
////    ret = qdb_statement(dbhandle, "SELECT vitalperiodic FROM('%q)", vital_col);
//    ret = qdb_statement(dbhandle, "SELECT * FROM patient;");
//    if (ret == -1) {
//        printf("db statement failed\n");
//        exit(-10);
//    }
//
////    errno = 0;
////    rows = qdb_rowchanges(dbhandle, NULL);
////    printf("rows = %d\n",rows);
////    if(errno != EOK){
////        printf("rows not ok\n");
////    }
//
//    result = qdb_getresult(dbhandle);
//    if (result == NULL) {
//        printf("Error:%s\n", strerror(errno));
//        exit(-10);
//    }
//
//    qdb_printmsg(stdout, result, QDB_FORMAT_COLUMN);
//}
