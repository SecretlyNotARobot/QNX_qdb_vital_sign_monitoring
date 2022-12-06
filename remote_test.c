
#define HTTP_IMPLEMENTATION
#define CHEAPDB_SERVER_URL "http://cheapdb.azurewebsites.net/db/post"
#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "includes/http.h"

// sends query, returns the HTTP code on a successful send or -1 on an error
int sendQuery(char *query, int querySize, char *response, int responseSize){
	//char requestBody[512];
	//sprintf(requestBody, "{\"query\":\"%s\"}", query);
	//printf("%s\n", requestBody);
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
			while( status == HTTP_STATUS_PENDING )
			{
				status = http_process( request );
				if( prev_size != (int) request->response_size )
				{
#ifdef DEBUG
					printf( "%d byte(s) received.\n", (int) request->response_size );
#endif

					prev_size = (int) request->response_size;
				}
			}

			if( status == HTTP_STATUS_FAILED )
			{
#ifdef DEBUG
				printf( "HTTP request failed (%d): %s.\n", request->status_code, request->reason_phrase );
#endif
				http_release( request );
				return request->status_code;
			}
#ifdef DEBUG
			printf( "\nContent type: %s\n\n%s\n", request->content_type, (char const*)request->response_data );
#endif
			if(request->response_size > responseSize)
				return -1;

			sprintf(response, "%s", request->response_data);

			http_release( request );

			return request->status_code;
}

int createTable(time_t *date){
	char tableId[10];
	struct tm currentDate;
	gmtime_r(date, &currentDate);

	sprintf(tableId, "t%.2i%.2i%.4i", currentDate.tm_mday, currentDate.tm_mon+1, currentDate.tm_year+1900);
	char tableExistsQuery[255];
	// SELECT object_id FROM sys.tables WHERE name ='{tableId}'
	sprintf(tableExistsQuery, "SELECT object_id FROM sys.tables WHERE name='%s'", tableId);

	printf("%s\n", tableExistsQuery);

	char buf[256];

	if(sendQuery(tableExistsQuery, strlen(tableExistsQuery), buf, sizeof(buf)) != 200)
		return -1;

	if(strcmp("[]", buf) == 0){
		char createTableQuery[255];
		//CREATE TABLE {tableId} (PatientId varchar(16), Heartrate float(4), Temp float(4), Respiration float(4), Sao2 float(4), BloodPressureS float(4))
		sprintf(createTableQuery, "CREATE TABLE %s (PatientId varchar(16), Heartrate float(4), Temp float(4), Respiration float(4), Sao2 float(4), BloodPressureS float(4));", tableId);

		printf("rec code %i, %s\n", sendQuery(createTableQuery, strlen(createTableQuery), buf, sizeof(buf)), buf);
	}


	return 0;
}

int appendUser(time_t * date){
	
	return 0;
}

int main(void){
	time_t currentTime;
	time(&currentTime);
	createTable(&currentTime);
	
	return 0;
}
