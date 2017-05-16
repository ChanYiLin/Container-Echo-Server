#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <errno.h>

#include "msg_helper.h"

int main(int argc,char **argv)
{
	int msgqid,msgqid2, rc;
	struct msg_buf msg;

	msgqid = msgget(MAGIC, MSGPERM|IPC_CREAT);
	if (msgqid < 0) {
		perror(strerror(errno));
		printf("failed to create message queue with msgqid = %d\n", msgqid);
		return 1;
	}

	msgqid2 = msgget(MAGIC2, 0);
	if (msgqid < 0) {
		perror(strerror(errno));
		printf("failed to create message queue with msgqid = %d\n", msgqid);
		return 1;
	}

	// 1. receive message from bridge through ipc MAGIC
	// 2. send the message to bridge through ipc MAGIC2

	while(1){

		//receive message  from bridge
		msg.mtype = 0;
		rc = msgrcv(msgqid, &msg, sizeof(msg.mtext), 0, 0); 
		if (rc < 0) {
			perror( strerror(errno) );
			printf("msgrcv failed, rc=%d\n", rc);
			return 1;
		} 
		printf("Recv from bridge: %s\n",msg.mtext);

		//send message to bridge
		msg.mtype = 1;
		rc = msgsnd(msgqid2, &msg, sizeof(msg.mtext), 0);
		if (rc < 0) {
			perror( strerror(errno) );
			printf("msgsnd failed, rc = %d\n", rc);
			return 1;
		}
		/*
			rc = msgctl(msgqid, IPC_RMID, NULL);
			if (rc < 0) {
				perror( strerror(errno) );
				printf("msgctl (return queue) failed, rc=%d\n", rc);
				return 1;
			}
		*/
	}
	


	return 0;
}