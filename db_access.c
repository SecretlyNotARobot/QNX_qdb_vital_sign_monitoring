#include "db_access.h"

//this is included here instead of in the header so the header can be compiled in to client processes.
#define HTTP_IMPLEMENTATION
#include "includes/http.h"


// sends query, returns the HTTP code on a successful send or -1 on an error
int sendQuery(char *query, int querySize, char *response, int responseSize){
	http_t* request = http_post(CHEAPDB_SERVER_URL, query, querySize, NULL);

		if( !request )
			{
#ifdef DEBUG
				printf( "Invalid request.\n" );
#endif
				return -1;
			}

		http_status_t status = HTTP_STATUS_PENDING;
		int prev_size = -1;
		while( status == HTTP_STATUS_PENDING ){
			status = http_process( request );
			if( prev_size != (int) request->response_size )
			{
#ifdef DEBUG
				printf( "%d byte(s) received.\n", (int) request->response_size );
#endif

				prev_size = (int) request->response_size;
			}
			// usleep(8);
		}


		if( status == HTTP_STATUS_FAILED ){
#ifdef DEBUG
			printf( "HTTP request failed (%d): %s.\n", request->status_code, request->reason_phrase );
#endif
			http_release( request );
			return request->status_code;
		}
#ifdef DEBUG
		printf( "\nContent type: %s\n\n%s\n", request->content_type, (char const*)request->response_data );
#endif
		if(responseSize != 0){
			if(request->response_size > responseSize)
				return -1;

			sprintf(response, "%s", request->response_data);
		}
		http_release( request );

		return request->status_code;

}

int sendQueryAsyncInternal(void *arg){
	//printf("In thread\n");
	sendQuery(arg, strlen(arg), NULL, 0);
	return 0;
}

int sendQueryAsync(char *query, int querySize){
	pthread_t thread;
	pthread_attr_t attr;
	struct sched_param param;
	param.sched_priority = 5;

	pthread_attr_init( &attr );
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedparam (&attr, &param);
	pthread_attr_setschedpolicy (&attr, SCHED_RR);

	pthread_create(&thread, &attr, &sendQueryAsyncInternal, query);

	pthread_detach(thread);
}

// Creates table with ID "tYYYYMMDD" or something. must be called to populate tableID.
int createTable(time_t *date){
	struct tm currentDate;
	gmtime_r(date, &currentDate);

	sprintf(tableId, "t%.2i%.2i%.4i", currentDate.tm_mday, currentDate.tm_mon+1, currentDate.tm_year+1900);
	char tableExistsQuery[255];
	// SELECT object_id FROM sys.tables WHERE name ='{tableId}'
	sprintf(tableExistsQuery, "SELECT object_id FROM sys.tables WHERE name='%s'", tableId);

#ifdef DEBUG
	printf("%s\n", tableExistsQuery);
#endif

	char buf[256];
	if(sendQuery(tableExistsQuery, strlen(tableExistsQuery), buf, sizeof(buf)) != 200)
		return -1;

	if(strcmp("[]", buf) == 0){
		char createTableQuery[255];
		//CREATE TABLE {tableId} (PatientId varchar(16), Heartrate float(4), Temp float(4), Respiration float(4), Sao2 float(4), BloodPressureS float(4))
		sprintf(createTableQuery, "CREATE TABLE %s (PatientId varchar(16), Heartrate float(4), Temp float(4), Respiration float(4), Sao2 float(4), BloodPressureS float(4), Mews int);", tableId);
#ifdef DEBUG
		printf("rec code %i, %s\n", sendQuery(createTableQuery, strlen(createTableQuery), buf, sizeof(buf)), buf);
#endif
	}


	return 0;
}

int createUser(char* Uuid){
	if(Uuid != 0 && strlen(Uuid) == 16){
		sprintf(userId, "%s", Uuid);
	}
	else{
		char buf[256];
		const char * query = "SELECT NEWID()";
		if(sendQuery(query, strlen(query)+1, buf, sizeof(buf)) == 200 ){
#ifdef DEBUG
			printf("newid(): %s\n", buf);
#endif
			int i = 0;
			for(int j=0; j<16; j++){
				while(!isalnum(buf[i])){
					i++;
				}
				userId[j] = buf[i];
				i++;
			}
#ifdef DEBUG
			printf("userid: %s\n", userId);
#endif


		}
		else
			return -1;
	}
	return 0;
}

int uploadFrame(float heartrate, float temp, float respiration, float sao2, float bloodPressureS, int mews){
	char query[512];
	sprintf(query, "INSERT INTO %s VALUES('%s', %f, %f, %f, %f, %f, %i);", tableId, userId, heartrate, temp, respiration, sao2, bloodPressureS, mews);
#ifdef DEBUG
	printf("insert query: %s\n");
#endif
	char * buf[256];
	sendQueryAsync(query, strlen(query));
	return 0;
}

const char* getTableId(){
	const char *ret = tableId;
	return ret;
}

const char* getUserId(){
	const char *ret = userId;
	return ret;
}
