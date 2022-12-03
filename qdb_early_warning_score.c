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




int main(void) {

	myMessage_t msg;
	int rcvid, msgid;
	name_attach_t *attach_temp;
	name_attach_t *attach_heartrate;
	char *success_rsp = "success";

	float temp;
	int heartrate;

	//create all channels
	attach_temp = name_attach(NULL, "temp", 0);
	attach_heartrate = name_attach(NULL, "heartrate", 0);

    //the server should keep receiving, processing and replying to messages
	while(1)
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
					printf("temp is gone\n"); break;
				default:
					printf("code is = %d  |  value = %d\n", msg.pulse.code, msg.pulse.value.sival_int); break;
			}
		}
		else if (rcvid > 0){
			//get vital data, store it ... assume it's the correct data type (it should be)
			temp = msg.vmsg.vital_data;
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
					printf("heartrate is gone\n"); break;
				default:
					printf("code is = %d  |  value = %d\n", msg.pulse.code, msg.pulse.value.sival_int); break;
			}
		}
		else if (rcvid > 0){
			//get vital data, store it ... assume it's the correct data type (it should be)
			heartrate = msg.vmsg.vital_data;
			msgid = MsgReply(rcvid, 1, &success_rsp, sizeof(success_rsp));
			if(msgid == -1){
				perror("MsgReply()");
				exit(-1);
			}
		}

		printf("temperature = %2.2f --- heartrate = %d\n", temp, heartrate);
	}

	//remove the name from the namespace and destroy the channel
	name_detach(attach_temp, 0);
	name_detach(attach_heartrate, 0);

return 0;
}
