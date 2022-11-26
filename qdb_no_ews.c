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
#define NUMTHREADS      2
char *num_of_retrievals = "2";

void* temp_vital(void*);
void* respiration_vital(void*);
void* heartrate_vital(void*);
void* sao2_vital(void*);
void* blood_pressure_vital(void*);
static int callback(void*, int, char**, char**);

int main(int argc, char **argv) {
	pthread_t threads[NUMTHREADS];

	//Get sched and prioirities set
	struct sched_param schedparam1;
	pthread_attr_t attr1;
	schedparam1.sched_priority = 5;
	pthread_attr_init(&attr1);
	pthread_attr_setinheritsched(&attr1, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr1, SCHED_RR);
	pthread_attr_setschedparam(&attr1, &schedparam1);

	struct sched_param schedparam2;
	pthread_attr_t attr2;
	schedparam2.sched_priority = 5;
	pthread_attr_init(&attr2);
	pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr2, SCHED_RR);
	pthread_attr_setschedparam(&attr2, &schedparam2);

	struct sched_param schedparam3;
	pthread_attr_t attr3;
	schedparam3.sched_priority = 5;
	pthread_attr_init(&attr3);
	pthread_attr_setinheritsched(&attr3, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr3, SCHED_RR);
	pthread_attr_setschedparam(&attr3, &schedparam3);

	struct sched_param schedparam4;
	pthread_attr_t attr4;
	schedparam4.sched_priority = 5;
	pthread_attr_init(&attr4);
	pthread_attr_setinheritsched(&attr4, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr4, SCHED_RR);
	pthread_attr_setschedparam(&attr4, &schedparam4);

	struct sched_param schedparam5;
	pthread_attr_t attr5;
	schedparam5.sched_priority = 5;
	pthread_attr_init(&attr5);
	pthread_attr_setinheritsched(&attr5, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr5, SCHED_RR);
	pthread_attr_setschedparam(&attr5, &schedparam5);


	pthread_create(&threads[0], &attr1, temp_vital, NULL);
	pthread_create(&threads[1], &attr2, respiration_vital, NULL);
	pthread_create(&threads[2], &attr3, heartrate_vital, NULL);
	pthread_create(&threads[3], &attr4, sao2_vital, NULL);
	pthread_create(&threads[4], &attr5, blood_pressure_vital, NULL);

    for (int i = 0; i < NUMTHREADS; i++) {
        void *status;
        if(pthread_join(threads[i], &status) != EOK){
        	printf("error joining\n");
        	exit(-1);
        }
    }

	return EXIT_SUCCESS;
}

//**************
//**************
//METHODS
//**************
//**************

//Callback
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	//Get column value
	float val = atof(argv[0]);
	//send it to remote database
	printf("%s = %2.2f\n", azColName[0], val);
    return 0;
}

//Callback for blood pressure
static int callback_bp(void *NotUsed, int argc, char **argv, char **azColName) {
	//Get column values - systolic and diastolic
	float systolic = atof(argv[0]);
	float diastolic = atof(argv[1]);
	//send it to remote database
	printf("%s = %2.2f --- %s = %2.2f\n", azColName[0], systolic, azColName[1], diastolic);
    return 0;
}

//Open database to get temperature vitals
void* temp_vital(void* arg){
	clock_t start, end;
	double cpu_time_used;
	int rc; //return code
	sqlite3 *handle;
	char select[100] = "select temperature from vitalperiodic limit ";
	strcat(select, num_of_retrievals);
	char *errmsg;

	//Start clock
	start = clock();

	//open DB
	sqlite3_open("/tmp/eicu_v2_0_1_copy.sqlite3", &handle);

	//Wrapper function that does sqlite3_prepare(), sqlite3_step(), sqlite3_column(),
	//	and sqlite3_finalize() for one or more SQL statements
	rc = sqlite3_exec(handle, select, callback, NULL, &errmsg);

	//Stop clock - calculate runtime
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	//Error Checking
	if (rc == SQLITE_ERROR) {
	      fprintf(stderr, "Error executing SELECT statement: %s\n", errmsg);
	      return NULL;
	   }
	else {
		printf("Successfully retrieved %s temps from database! in %2.8f s\n", num_of_retrievals, cpu_time_used);
	}
	return NULL;
}

//Open database to get respiration vitals
void* respiration_vital(void* arg){
	clock_t start, end;
	double cpu_time_used;
	int rc; //return code
	sqlite3 *handle;
	char select[100] = "select respiration from vitalperiodic limit ";
	strcat(select, num_of_retrievals);
	char *errmsg;

	//Start clock
	start = clock();

	//open DB
	sqlite3_open("/tmp/eicu_v2_0_1_copy.sqlite3", &handle);

	//Wrapper function that does sqlite3_prepare(), sqlite3_step(), sqlite3_column(),
	//	and sqlite3_finalize() for one or more SQL statements
	rc = sqlite3_exec(handle, select, callback, NULL, &errmsg);

	//Stop clock - calculate runtime
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	//Error Checking
	if (rc == SQLITE_ERROR) {
	      fprintf(stderr, "Error executing SELECT statement: %s\n", errmsg);
	      return NULL;
	   }
	else {
		printf("Successfully retrieved %s respirations from database! in %2.8f s\n", num_of_retrievals, cpu_time_used);
	}
	return NULL;
}

//Open database to get heartrate vitals
void* heartrate_vital(void* arg){
	clock_t start, end;
	double cpu_time_used;
	int rc; //return code
	sqlite3 *handle;
	char select[100] = "select heartrate from vitalperiodic limit ";
	strcat(select, num_of_retrievals);
	char *errmsg;

	//Start clock
	start = clock();

	//open DB
	sqlite3_open("/tmp/eicu_v2_0_1_copy.sqlite3", &handle);

	//Wrapper function that does sqlite3_prepare(), sqlite3_step(), sqlite3_column(),
	//	and sqlite3_finalize() for one or more SQL statements
	rc = sqlite3_exec(handle, select, callback, NULL, &errmsg);

	//Stop clock - calculate runtime
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	//Error Checking
	if (rc == SQLITE_ERROR) {
	      fprintf(stderr, "Error executing SELECT statement: %s\n", errmsg);
	      return NULL;
	   }
	else {
		printf("Successfully retrieved %s heartrates from database! in %2.8f s\n", num_of_retrievals, cpu_time_used);
	}
	return NULL;
}

//Open database to get sao2 vitals
void* sao2_vital(void* arg){
	clock_t start, end;
	double cpu_time_used;
	int rc; //return code
	sqlite3 *handle;
	char select[100] = "select sao2 from vitalperiodic limit ";
	strcat(select, num_of_retrievals);
	char *errmsg;

	//Start clock
	start = clock();

	//open DB
	sqlite3_open("/tmp/eicu_v2_0_1_copy.sqlite3", &handle);

	//Wrapper function that does sqlite3_prepare(), sqlite3_step(), sqlite3_column(),
	//	and sqlite3_finalize() for one or more SQL statements
	rc = sqlite3_exec(handle, select, callback, NULL, &errmsg);

	//Stop clock - calculate runtime
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	//Error Checking
	if (rc == SQLITE_ERROR) {
	      fprintf(stderr, "Error executing SELECT statement: %s\n", errmsg);
	      return NULL;
	   }
	else {
		printf("Successfully retrieved %s sao2 from database! in %2.8f s\n", num_of_retrievals, cpu_time_used);
	}
	return NULL;
}

//Open database to get blood pressure vitals
void* blood_pressure_vital(void* arg){
	clock_t start, end;
	double cpu_time_used;
	int rc; //return code
	sqlite3 *handle;
	char select[100] = "select systemicsystolic, systemicdiastolic from vitalperiodic limit ";
	strcat(select, num_of_retrievals);
	char *errmsg;

	//Start clock
	start = clock();

	//open DB
	sqlite3_open("/tmp/eicu_v2_0_1_copy.sqlite3", &handle);

	//Wrapper function that does sqlite3_prepare(), sqlite3_step(), sqlite3_column(),
	//	and sqlite3_finalize() for one or more SQL statements
	rc = sqlite3_exec(handle, select, callback_bp, NULL, &errmsg);

	//Stop clock - calculate runtime
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	//Error Checking
	if (rc == SQLITE_ERROR) {
	      fprintf(stderr, "Error executing SELECT statement: %s\n", errmsg);
	      return NULL;
	   }
	else {
		printf("Successfully retrieved %s blood pressures from database! in %2.8f s\n", num_of_retrievals, cpu_time_used);
	}
	return NULL;
}
