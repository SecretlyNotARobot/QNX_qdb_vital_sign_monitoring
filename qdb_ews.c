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
#include "db_access.h" //defines functions for working with remote DB

/** SQLite3 API Documentation:
 *
 * https://www.sqlite.org/cintro.html
 * https://www.sqlite.org/rescode.html#ok
 *
 */
#define NUMTHREADS      5

//Globals
char *num_of_retrievals = "1000";
int heartrate_coid;
int temp_coid;
int respiration_coid;
int sao2_coid;
int blood_pressure_coid;

void createPriorityThread(pthread_t thread_id[] ,int index, int priority, void (*func));
void ReceiveMessageOrPulseAndUpdateData(int, int*, char*, float*, int*);
void* temp_vital(void*);
void* respiration_vital(void*);
void* heartrate_vital(void*);
void* sao2_vital(void*);
void* blood_pressure_vital(void*);
static int callback_temp(void*, int, char**, char**);
static int callback_hr(void*, int, char**, char**);
static int callback_resp(void*, int, char**, char**);
static int callback_sao2(void*, int, char**, char**);
static int callback_bp(void*, int, char**, char**);
int calculate_ews(int, int, int, int, int);

int main(int argc, char **argv) {
	time_t startTimestamp;
	time(&startTimestamp);
	//Variables for EWS
	name_attach_t *attach_temp;
	name_attach_t *attach_heartrate;
	name_attach_t *attach_respiration;
	name_attach_t *attach_sao2;
	name_attach_t *attach_blood_pressure;
	int ews_temp, ews_heartrate, ews_respiration;
	int ews_sao2, ews_blood_pressure, ews_total;

	//Vital Variables
	int num_vitals = NUMTHREADS;
	float temp, heartrate, respiration, sao2, blood_pressure;
	char* pulse_msg_temp = "temp is gone\n";
	char* pulse_msg_hr = "heartrate is gone\n";
	char* pulse_msg_resp = "respiration is gone\n";
	char* pulse_msg_sao2 = "sao2 is gone\n";
	char* pulse_msg_bp = "blood pressure is gone\n";

	//Set up remote DB
	time_t currentTime;
	time(&currentTime);
	createTable(&currentTime);
	createUser(NULL);

	//create all channels
	attach_temp = name_attach(NULL, "temp", 0);
	attach_heartrate = name_attach(NULL, "heartrate", 0);
	attach_respiration = name_attach(NULL, "respiration", 0);
	attach_sao2 = name_attach(NULL, "sao2", 0);
	attach_blood_pressure = name_attach(NULL, "bloodpressure", 0);

	//Setting up the threads for each vital sign
	pthread_t thread_id[NUMTHREADS];

	// Create threads 1-5 with correct priorities
	for (int i = 0; i < NUMTHREADS; i++) {
		switch (i) {
		case 0:
				createPriorityThread(thread_id, i, 5, temp_vital);
				break;

		case 1:
				createPriorityThread(thread_id, i, 5, heartrate_vital);
				break;

		case 2:
				createPriorityThread(thread_id, i, 5, respiration_vital);
				break;

		case 3:
				createPriorityThread(thread_id, i, 5, sao2_vital);
				break;

		case 4:
				createPriorityThread(thread_id, i, 5, blood_pressure_vital);
				break;
		}
	}


    //the server should keep receiving, processing and replying to messages
	while(num_vitals > 0)
	{

		//Receive msg or pulse from temp
		ReceiveMessageOrPulseAndUpdateData(attach_temp->chid, &num_vitals, pulse_msg_temp, &temp, &ews_temp);

		//Receive msg or pulse from heartrate
		ReceiveMessageOrPulseAndUpdateData(attach_heartrate->chid, &num_vitals, pulse_msg_hr, &heartrate, &ews_heartrate);

		//Receive msg or pulse from respiration
		ReceiveMessageOrPulseAndUpdateData(attach_respiration->chid, &num_vitals, pulse_msg_resp, &respiration, &ews_respiration);

		//Receive msg or pulse from sao2
		ReceiveMessageOrPulseAndUpdateData(attach_sao2->chid, &num_vitals, pulse_msg_sao2, &sao2, &ews_sao2);

		//Receive msg or pulse from blood pressure
		ReceiveMessageOrPulseAndUpdateData(attach_blood_pressure->chid, &num_vitals, pulse_msg_bp, &blood_pressure, &ews_blood_pressure);

		//Calculate EWS and print it
		ews_total = calculate_ews(ews_heartrate, ews_temp, ews_respiration, ews_sao2, ews_blood_pressure);
		printf("Calculated aggregate EWS = %d\n", ews_total);
		//Send to online database
		uploadFrame(temp, heartrate, respiration, sao2, blood_pressure, ews_total);

	}

	//remove the names from the namespace and destroy the channels
	name_detach(attach_temp, 0);
	name_detach(attach_heartrate, 0);
	name_detach(attach_respiration, 0);
	name_detach(attach_sao2, 0);
	name_detach(attach_blood_pressure, 0);

    for (int i = 0; i < NUMTHREADS; i++) {
        void *status;
        if(pthread_join(thread_id[i], &status) != EOK){
        	printf("error joining\n");
        	exit(-1);
        }
    }

	time_t endTimestamp;
	time(&endTimestamp);
	printf("%s UID\n", getUserId());
	printf("start time: %i end time: %i\nelapsed: %i", startTimestamp, endTimestamp, endTimestamp-startTimestamp);


	sleep(5);
	return EXIT_SUCCESS;
}

//**************
//**************
//METHODS
//**************
//**************

void createPriorityThread(pthread_t thread_id[] ,int index, int priority, void (*func)) {
	pthread_attr_t attr;
	struct sched_param param; //Structure that describes scheduling parameters

	// Initialize attr structure and use its scheduling policy
	pthread_attr_init( &attr );
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	// Set the scheduling parameters, policy, and priority
	param.sched_priority = priority;
	pthread_attr_setschedparam (&attr, &param);
	pthread_attr_setschedpolicy (&attr, SCHED_RR);

	pthread_create (&thread_id[index], &attr, func, NULL); // Create the thread
}

//Calculate early warning score
int calculate_ews(int hr_ews, int tmp_ews, int resp_ews, int sao2_ews, int bp_ews) {
	int aggregate_score = hr_ews + tmp_ews + resp_ews + sao2_ews + bp_ews;
	return aggregate_score;
}

//Callback - temp
static int callback_temp(void *NotUsed, int argc, char **argv, char **azColName) {
	char rmsg[MAX_STRING_LEN];
	struct get_vital_msg_ews vmsg;
	float val;
	long msg_send_id;

	//Get column value
	val = atof(argv[0]);


	//Calculate EWS
	if (val <= 35.0) {
		vmsg.ews = 3;
	}
	else if (val <= 36.0) {
			vmsg.ews = 1;
		}
	else if (val <= 38.0) {
			vmsg.ews = 0;
		}
	else if (val <= 39.0) {
			vmsg.ews = 1;
		}
	else {
		vmsg.ews = 2;
	}

	//Send it to EWS calculator
	vmsg.type = TEMPERATURE;
	vmsg.vital_data = val;
	msg_send_id = MsgSend(temp_coid, &vmsg, sizeof(vmsg), rmsg, sizeof(rmsg));
	if(msg_send_id == -1){
		perror("MsgSend()");
		exit(-1);
	}

	//TODO: send it to remote database
	printf("%s = %2.2f\n", azColName[0], val);

    return 0;
}

//Callback - heartrate
static int callback_hr(void *NotUsed, int argc, char **argv, char **azColName) {
	char rmsg[MAX_STRING_LEN];
	struct get_vital_msg_ews vmsg;
	float val;
	long msg_send_id;

	//Get column value
	val = atof(argv[0]);

	//Calculate EWS
	if (val <= 40) {
		vmsg.ews = 3;
	}
	else if (val <= 50) {
			vmsg.ews = 1;
		}
	else if (val <= 90) {
			vmsg.ews = 0;
		}
	else if (val <= 110) {
			vmsg.ews = 1;
		}
	else if (val <= 130) {
			vmsg.ews = 2;
		}
	else {
		vmsg.ews = 3;
	}

	//Send it to EWS calculator
	vmsg.type = HEARTRATE;
	vmsg.vital_data = val;
	msg_send_id = MsgSend(heartrate_coid, &vmsg, sizeof(vmsg), rmsg, sizeof(rmsg));
	if(msg_send_id == -1){
		perror("MsgSend()");
		exit(-1);
	}

	//TODO: send it to remote database
	printf("%s = %2.2f\n", azColName[0], val);

    return 0;
}

//Callback - resp
static int callback_resp(void *NotUsed, int argc, char **argv, char **azColName) {
	char rmsg[MAX_STRING_LEN];
	struct get_vital_msg_ews vmsg;
	float val;
	long msg_send_id;

	//Get column value
	val = atof(argv[0]);

	//Calculate Respiration EWS
	if (val <= 8) {
		vmsg.ews = 3;
	}
	else if (val <= 11) {
			vmsg.ews = 1;
		}
	else if (val <= 20) {
			vmsg.ews = 0;
		}
	else if (val <= 24) {
			vmsg.ews = 2;
		}
	else {
		vmsg.ews = 3;
	}

	//Send it to EWS calculator
	vmsg.type = RESPIRATION;
	vmsg.vital_data = val;
	msg_send_id = MsgSend(respiration_coid, &vmsg, sizeof(vmsg), rmsg, sizeof(rmsg));
	if(msg_send_id == -1){
		perror("MsgSend()");
		exit(-1);
	}

	//TODO: send it to remote database
	printf("%s = %2.2f\n", azColName[0], val);

    return 0;
}

//Callback - sao2
static int callback_sao2(void *NotUsed, int argc, char **argv, char **azColName) {
	char rmsg[MAX_STRING_LEN];
	struct get_vital_msg_ews vmsg;
	float val;
	long msg_send_id;

	//Get column value
	val = atof(argv[0]);
	//TODO: calculate sao2 ews

	//Calculate SAO2 EWS
	if (val <= 91) {
		vmsg.ews = 3;
	}
	else if (val <= 93) {
			vmsg.ews = 2;
		}
	else if (val <= 95) {
			vmsg.ews = 1;
		}
	else {
		vmsg.ews = 0;
	}

	//Send it to EWS calculator
	vmsg.type = SAO2;
	vmsg.vital_data = val;
	msg_send_id = MsgSend(sao2_coid, &vmsg, sizeof(vmsg), rmsg, sizeof(rmsg));
	if(msg_send_id == -1){
		perror("MsgSend()");
		exit(-1);
	}

	//TODO: send it to remote database
	printf("%s = %2.2f\n", azColName[0], val);

    return 0;
}

//Callback - bp
static int callback_bp(void *NotUsed, int argc, char **argv, char **azColName) {
	char rmsg[MAX_STRING_LEN];
	struct get_vital_msg_ews vmsg;
	float systolic;
	long msg_send_id;

	//Get column value
	systolic = atof(argv[0]);

	//Calculate Systolic Blood Pressure EWS
	if (systolic <= 90) {
		vmsg.ews = 3;
	}
	else if (systolic <= 100) {
			vmsg.ews = 2;
		}
	else if (systolic <= 110) {
			vmsg.ews = 1;
		}
	else if (systolic <= 219) {
			vmsg.ews = 0;
		}
	else {
		vmsg.ews = 3;
	}
	//Send it to EWS calculator
	vmsg.type = BLOODPRESSURE;
	vmsg.vital_data = systolic;
	msg_send_id = MsgSend(blood_pressure_coid, &vmsg, sizeof(vmsg), rmsg, sizeof(rmsg));
	if(msg_send_id == -1){
		perror("MsgSend()");
		exit(-1);
	}

	//TODO: send it to remote database
	printf("%s = %2.2f\n", azColName[0], systolic);

    return 0;
}


//Open database to get temperature vitals
void* temp_vital(void* arg){
	clock_t start, end;
	double cpu_time_used;
	int rc; //return code
	sqlite3 *handle;
	char select[100] = "select temperature from vitalperiodic where patientunitstayid=2559053 limit ";
	strcat(select, num_of_retrievals);
	char *errmsg;

	//Start clock
	start = clock();

	//open connection to ews
	temp_coid = name_open("temp", 0);

	//open DB
	sqlite3_open("/tmp/eicu_v2_0_1_copy.sqlite3", &handle);

	//Wrapper function that does sqlite3_prepare(), sqlite3_step(), sqlite3_column(),
	//	and sqlite3_finalize() for one or more SQL statements
	rc = sqlite3_exec(handle, select, callback_temp, NULL, &errmsg);

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

	rc = MsgSendPulse(temp_coid, -1, _PULSE_CODE_DISCONNECT, 0);
	if(rc == -1){
		perror("MsgSendPulse()");
		exit(-1);
	}

	return NULL;
}


//Open database to get heartrate vitals
void* heartrate_vital(void* arg){
	clock_t start, end;
	double cpu_time_used;
	int rc; //return code
	sqlite3 *handle;
	char select[100] = "select heartrate from vitalperiodic where patientunitstayid=2559053 limit ";
	strcat(select, num_of_retrievals);
	char *errmsg;

	//Start clock
	start = clock();

	//open connection to ews
	heartrate_coid = name_open("heartrate", 0);

	//open DB
	sqlite3_open("/tmp/eicu_v2_0_1_copy.sqlite3", &handle);

	//Wrapper function that does sqlite3_prepare(), sqlite3_step(), sqlite3_column(),
	//	and sqlite3_finalize() for one or more SQL statements
	rc = sqlite3_exec(handle, select, callback_hr, NULL, &errmsg);

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

	rc = MsgSendPulse(heartrate_coid, -1, _PULSE_CODE_DISCONNECT, 0);
	if(rc == -1){
		perror("MsgSendPulse()");
		exit(-1);
	}

	return NULL;
}


//Open database to get respiration vitals
void* respiration_vital(void* arg){
	clock_t start, end;
	double cpu_time_used;
	int rc;//return code
	sqlite3 *handle;
	char select[100] = "select respiration from vitalperiodic where patientunitstayid=2559053 limit ";
	strcat(select, num_of_retrievals);
	char *errmsg;

	//Start clock
	start = clock();

	//open connection to ews
	respiration_coid = name_open("respiration", 0);

	//open DB
	sqlite3_open("/tmp/eicu_v2_0_1_copy.sqlite3", &handle);

	//Wrapper function that does sqlite3_prepare(), sqlite3_step(), sqlite3_column(),
	//	and sqlite3_finalize() for one or more SQL statements
	rc = sqlite3_exec(handle, select, callback_resp, NULL, &errmsg);

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

	rc = MsgSendPulse(respiration_coid, -1, _PULSE_CODE_DISCONNECT, 0);
	if(rc == -1){
		perror("MsgSendPulse()");
		exit(-1);
	}

	return NULL;
}



//Open database to get sao2 vitals
void* sao2_vital(void* arg){
	clock_t start, end;
	double cpu_time_used;
	int rc; //return code
	sqlite3 *handle;
	char select[100] = "select sao2 from vitalperiodic where patientunitstayid=2559053 limit ";
	strcat(select, num_of_retrievals);
	char *errmsg;

	//Start clock
	start = clock();

	//open connection to ews
	sao2_coid = name_open("sao2", 0);

	//open DB
	sqlite3_open("/tmp/eicu_v2_0_1_copy.sqlite3", &handle);

	//Wrapper function that does sqlite3_prepare(), sqlite3_step(), sqlite3_column(),
	//	and sqlite3_finalize() for one or more SQL statements
	rc = sqlite3_exec(handle, select, callback_sao2, NULL, &errmsg);

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

	rc = MsgSendPulse(sao2_coid, -1, _PULSE_CODE_DISCONNECT, 0);
	if(rc == -1){
		perror("MsgSendPulse()");
		exit(-1);
	}

	return NULL;
}

//Open database to get blood pressure vitals
void* blood_pressure_vital(void* arg){
	clock_t start, end;
	double cpu_time_used;
	int rc; //return code
	sqlite3 *handle;
	char select[100] = "select systemicsystolic from vitalperiodic where patientunitstayid=2559053 limit ";
	strcat(select, num_of_retrievals);
	char *errmsg;

	//Start clock
	start = clock();

	//open connection to ews
	blood_pressure_coid = name_open("bloodpressure", 0);

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

	rc = MsgSendPulse(blood_pressure_coid, -1, _PULSE_CODE_DISCONNECT, 0);
	if(rc == -1){
		perror("MsgSendPulse()");
		exit(-1);
	}

	return NULL;
}


//Receive a Message and update appropriate vital data
void ReceiveMessageOrPulseAndUpdateData(int chid, int* num_vitals, char* pulse_msg, float* data, int* ews){
	int rcvid, msgid;
	char *success_rsp = "success";
	myMessage_t msg;

	//code to receive msg or pulse from blood pressure
	rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
	if(rcvid == -1){
		perror("MsgReceive()");
		exit(-1);
	}
	//check if it was a pulse or a message
	if(rcvid == 0){
		switch(msg.pulse.code){
			case _PULSE_CODE_DISCONNECT:
				*num_vitals = *num_vitals - 1;
				printf("%s", pulse_msg); break;
			default:
				printf("code is = %d  |  value = %d\n", msg.pulse.code, msg.pulse.value.sival_int); break;
		}
	}
	else if (rcvid > 0){
		//get vital data, store it ... assume it's the correct data type (it should be)
		*data = msg.vmsg.vital_data;
		*ews = msg.vmsg.ews;
		msgid = MsgReply(rcvid, 1, &success_rsp, sizeof(success_rsp));
		if(msgid == -1){
			perror("MsgReply()");
			exit(-1);
		}
	}
}







