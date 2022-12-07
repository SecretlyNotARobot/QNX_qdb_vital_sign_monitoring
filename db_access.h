#define CHEAPDB_SERVER_URL "http://cheapdb.azurewebsites.net/db/post"
//#define DEBUG

#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include <pthread.h>

int sendQueryAsync(char *query, int querySize);

int sendQuery(char *query, int querySize, char *response, int responseSize);

int createTable(time_t *date);

int createUser(char* Uuid);

int uploadFrame(float heartrate, float temp, float respiration, float sao2, float bloodPressureS, int mews);

static char userId[17];
static char tableId[10];

const char* getTableId();
const char* getUserId();
