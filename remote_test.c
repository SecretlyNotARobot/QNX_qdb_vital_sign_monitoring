#include <stdio.h>
#include <time.h>
#include "db_access.h"



int main(void){
	time_t currentTime;
	time(&currentTime);
	createTable(&currentTime);
	printf("Table ID: %s\n", getTableId());

	//passing null generates a GUID from the server
	createUser(NULL);

	createUser("AAAABBBBCCCCDDDD");
	printf("User ID: %s\n", getUserId());

	uploadFrame(1.1111f, 2.2222f, 55555.0f, 0.0011f, 6.0f, 99);

	return 0;
}
