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

//Globals
char *num_of_retrievals = "2";
int heartrate_coid;
int temp_coid;
int respiration_coid;
int sao2_coid;
int blood_pressure_coid;


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
	//Variables for EWS
	myMessage_t msg;
	int rcvid, msgid;
	name_attach_t *attach_temp;
	name_attach_t *attach_heartrate;
	name_attach_t *attach_respiration;
	name_attach_t *attach_sao2;
	name_attach_t *attach_blood_pressure;
	char *success_rsp = "success";
	int ews_temp;
	int ews_heartrate;
	int ews_respiration;
	int ews_sao2;
	int ews_blood_pressure;
	int ews_total;

	//Vital Variables
	int num_vitals = 3;
	float temp;
	float heartrate;
	float respiration;
	float sao2;
	float blood_pressure_s;
	float blood_pressure_d;

	//create all channels
	attach_temp = name_attach(NULL, "temp", 0);
	attach_heartrate = name_attach(NULL, "heartrate", 0);
	attach_respiration = name_attach(NULL, "respiration", 0);
	attach_sao2 = name_attach(NULL, "sao2", 0);
	attach_blood_pressure = name_attach(NULL, "bloodpressure", 0);

	//Setting up the threads for each vital sign
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
	pthread_create(&threads[1], &attr2, heartrate_vital, NULL);
	pthread_create(&threads[2], &attr3, respiration_vital, NULL);
	pthread_create(&threads[3], &attr4, sao2_vital, NULL);
	pthread_create(&threads[4], &attr5, blood_pressure_vital, NULL);




    //the server should keep receiving, processing and replying to messages
	while(num_vitals > 0)
	{
	  //code to receive msg or pulse from temp
		rcvid = MsgReceive(attach_temp->chid, &msg, sizeof(msg), NULL);
		if(rcvid == -1){
			perror("MsgReceive()");
			exit(-1);
		}
	  //check if it was a pulse or a message
		if(rcvid == 0){
			switch(msg.pulse.code){
				case _PULSE_CODE_DISCONNECT:
					num_vitals--;

					printf("temp is gone\n"); break;
				default:
					printf("code is = %d  |  value = %d\n", msg.pulse.code, msg.pulse.value.sival_int); break;
			}
		}
		else if (rcvid > 0){
			//get vital data, store it ... assume it's the correct data type (it should be)
			temp = msg.vmsg.vital_data;
			ews_temp = msg.vmsg.ews;
			msgid = MsgReply(rcvid, 1, &success_rsp, sizeof(success_rsp));
			if(msgid == -1){
				perror("MsgReply()");
				exit(-1);
			}
		}

	  //code to receive msg or pulse from heartrate
		rcvid = MsgReceive(attach_heartrate->chid, &msg, sizeof(msg), NULL);
		if(rcvid == -1){
			perror("MsgReceive()");
			exit(-1);
		}
	  //check if it was a pulse or a message
		if(rcvid == 0){
			switch(msg.pulse.code){
				case _PULSE_CODE_DISCONNECT:
					num_vitals--;
					printf("heartrate is gone\n"); break;
				default:
					printf("code is = %d  |  value = %d\n", msg.pulse.code, msg.pulse.value.sival_int); break;
			}
		}
		else if (rcvid > 0){
			//get vital data, store it ... assume it's the correct data type (it should be)
			heartrate = msg.vmsg.vital_data;
			ews_heartrate = msg.vmsg.ews;
			msgid = MsgReply(rcvid, 1, &success_rsp, sizeof(success_rsp));
			if(msgid == -1){
				perror("MsgReply()");
				exit(-1);
			}
		}

		//code to receive msg or pulse from respiration
		rcvid = MsgReceive(attach_respiration->chid, &msg, sizeof(msg), NULL);
		if(rcvid == -1){
			perror("MsgReceive()");
			exit(-1);
		}
	    //check if it was a pulse or a message
		if(rcvid == 0){
			switch(msg.pulse.code){
				case _PULSE_CODE_DISCONNECT:
					num_vitals--;
					printf("respiration is gone\n"); break;
				default:
					printf("code is = %d  |  value = %d\n", msg.pulse.code, msg.pulse.value.sival_int); break;
			}
		}
		else if (rcvid > 0){
			//get vital data, store it ... assume it's the correct data type (it should be)
			respiration = msg.vmsg.vital_data;
			ews_respiration = msg.vmsg.ews;
			msgid = MsgReply(rcvid, 1, &success_rsp, sizeof(success_rsp));
			if(msgid == -1){
				perror("MsgReply()");
				exit(-1);
			}
		}


		//code to receive msg or pulse from respiration
		rcvid = MsgReceive(attach_sao2->chid, &msg, sizeof(msg), NULL);
		if(rcvid == -1){
			perror("MsgReceive()");
			exit(-1);
		}
		//check if it was a pulse or a message
		if(rcvid == 0){
			switch(msg.pulse.code){
				case _PULSE_CODE_DISCONNECT:
					num_vitals--;
					printf("sao2 is gone\n"); break;
				default:
					printf("code is = %d  |  value = %d\n", msg.pulse.code, msg.pulse.value.sival_int); break;
			}
		}
		else if (rcvid > 0){
			//get vital data, store it ... assume it's the correct data type (it should be)
			sao2 = msg.vmsg.vital_data;
			ews_sao2 = msg.vmsg.ews;
			msgid = MsgReply(rcvid, 1, &success_rsp, sizeof(success_rsp));
			if(msgid == -1){
				perror("MsgReply()");
				exit(-1);
			}
		}


		//code to receive msg or pulse from respiration
		rcvid = MsgReceive(attach_blood_pressure->chid, &msg, sizeof(msg), NULL);
		if(rcvid == -1){
			perror("MsgReceive()");
			exit(-1);
		}
		//check if it was a pulse or a message
		if(rcvid == 0){
			switch(msg.pulse.code){
				case _PULSE_CODE_DISCONNECT:
					num_vitals--;
					printf("blood pressure is gone\n"); break;
				default:
					printf("code is = %d  |  value = %d\n", msg.pulse.code, msg.pulse.value.sival_int); break;
			}
		}
		else if (rcvid > 0){
			//get vital data, store it ... assume it's the correct data type (it should be)
			blood_pressure_s = msg.vmsg.vital_data;
			blood_pressure_d = msg.vmsg.vital_data_2;
			ews_blood_pressure = msg.vmsg.ews;
			msgid = MsgReply(rcvid, 1, &success_rsp, sizeof(success_rsp));
			if(msgid == -1){
				perror("MsgReply()");
				exit(-1);
			}
		}


		//Calculate EWS and print it
		ews_total = calculate_ews(ews_heartrate, ews_temp, ews_respiration, ews_sao2, ews_blood_pressure);
		printf("Calculated aggregate EWS = %d\n", ews_total);
		//TODO: Send to online database

	}

	//remove the name from the namespace and destroy the channel
	name_detach(attach_temp, 0);
	name_detach(attach_heartrate, 0);
	name_detach(attach_respiration, 0);
	name_detach(attach_sao2, 0);
	name_detach(attach_blood_pressure, 0);

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

//Calculate early warning score
int calculate_ews(int hr_ews, int tmp_ews, int resp_ews, int sao2_ews, int bp_ews) {
	int aggregate_score = hr_ews + tmp_ews + resp_ews + sao2_ews + bp_ews;
	return aggregate_score;
}

//Callback - temp
static int callback_temp(void *NotUsed, int argc, char **argv, char **azColName) {
	char rmsg[MAX_STRING_LEN];
	struct get_vital_msg vmsg;
	float val;
	long msg_send_id;

	//Get column value
	val = atof(argv[0]);
	//TODO: calculate temp ews

	//Send it to EWS calculator
	vmsg.type = TEMPERATURE;
	vmsg.vital_data = val;
	//TODO: include ews in send message
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
	struct get_vital_msg vmsg;
	float val;
	long msg_send_id;

	//Get column value
	val = atof(argv[0]);
	//TODO: calculate heartrate ews

	//Send it to EWS calculator
	vmsg.type = HEARTRATE;
	vmsg.vital_data = val;
	//TODO: include ews in send message
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
	struct get_vital_msg vmsg;
	float val;
	long msg_send_id;

	//Get column value
	val = atof(argv[0]);
	//TODO: calculate resp ews

	//Send it to EWS calculator
	vmsg.type = RESPIRATION;
	vmsg.vital_data = val;
	//TODO: include ews in send message
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
	struct get_vital_msg vmsg;
	float val;
	long msg_send_id;

	//Get column value
	val = atof(argv[0]);
	//TODO: calculate sao2 ews

	//Send it to EWS calculator
	vmsg.type = SAO2;
	vmsg.vital_data = val;
	//TODO: include ews in send message
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
	struct get_vital_msg vmsg;
	float systolic, diastolic;
	long msg_send_id;

	//Get column value
	systolic = atof(argv[0]);
	diastolic = atof(argv[1]);
	//TODO: calculate bloodpressure ews

	//Send it to EWS calculator
	vmsg.type = BLOODPRESSURE;
	vmsg.vital_data = systolic;
	vmsg.vital_data_2 = diastolic;
	//TODO: include ews in send message
	msg_send_id = MsgSend(blood_pressure_coid, &vmsg, sizeof(vmsg), rmsg, sizeof(rmsg));
	if(msg_send_id == -1){
		perror("MsgSend()");
		exit(-1);
	}

	//TODO: send it to remote database
	printf("%s = %2.2f ---- %s = %2.2f\n", azColName[0], systolic, azColName[1], diastolic);

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
	char select[100] = "select heartrate from vitalperiodic limit ";
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
	char select[100] = "select respiration from vitalperiodic limit ";
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
	char select[100] = "select sao2 from vitalperiodic limit ";
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
	char select[100] = "select systemicsystolic, systemicdiastolic from vitalperiodic limit ";
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
