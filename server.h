#include <stdint.h>
#include <sys/iomsg.h>
#include <sys/mman.h>

#define GET_SHMEM_MSG_TYPE (_IO_MAX+200)
#define CHANGED_SHMEM_MSG_TYPE (_IO_MAX+201)
#define RELEASE_SHMEM_MSG_TYPE (_IO_MAX+202)

#define SHMEM_SERVER_NAME "shmem_server"

#define MAX_STRING_LEN    256
#define TEMPERATURE (_IO_MAX+200)
#define HEARTRATE (_IO_MAX+201)
#define RESPIRATION (_IO_MAX+202)
#define SAO2 (_IO_MAX+203)
#define BLOODPRESSURE (_IO_MAX+204)

typedef struct get_vital_msg {
	uint16_t type;
	unsigned vital_data;
} get_vital_msg_t;

typedef struct get_vital_msg_ews {
	uint16_t type;
	unsigned vital_data;
	unsigned ews;
} get_vital_msg_ews_t;

typedef union
{
	struct _pulse pulse;
    struct get_vital_msg_ews vmsg;
} myMessage_t;

