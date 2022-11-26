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

typedef struct get_vital_msg {
	uint16_t type;
	unsigned vital_data;
} get_vital_msg_t;

