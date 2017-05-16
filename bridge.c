//bridge.c
//bridge.c

// 1. read and delete the client_message file 
// 2. send the message to server and receive the msg from server
// 3. create the bridge_message file and write the message into it.
// 4. after detecting the closing of the client_message file then open, read and delete the file
#define _GNU_SOURCE
#include <sys/inotify.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/errno.h>


#include <sched.h>
#include <fcntl.h>

#include "msg_helper.h"


#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

int main(int argc, char *argv[])
{

	//./bridge /proc/containerA_PID/ns/ipc /proc/containerB_PID/ns/mnt
	// A, server, use ipc
	// B, client, use mnt
 

	if(setns(open(argv[1],O_RDONLY),CLONE_NEWIPC)){
		printf("setns server fail\n");
		return 1;
	}
	if(setns(open(argv[2],O_RDONLY),CLONE_NEWNS)){
		printf("setns client fail\n");
		return 1;
	}

	system("rm -f /tmp/msg/server_msg");
	system("rm -f /tmp/msg/bridge_msg");

	char client_bridge_buf[500];
	char server_bridge_buf[500];

	int msgqid,msgqid2, rc;
	struct msg_buf msg;

	int inotifyFd, wd, j;
	char buf[BUF_LEN] __attribute__ ((aligned(8)));
	ssize_t numRead;
	char *p;
	struct inotify_event *event;

	inotifyFd = inotify_init();
	if (inotifyFd == -1) {
		perror(strerror(errno));
		printf("inotifyFd\n");
		return 1;
	}

	wd = inotify_add_watch(inotifyFd, "/tmp/msg/", IN_CLOSE_WRITE);
	if (wd == -1) {
		perror(strerror(errno));
		printf("inotify_add_watch\n");
		return 1;
	}

	while(1) {
		memset(client_bridge_buf, '\0', sizeof(client_bridge_buf));
		memset(server_bridge_buf, '\0', sizeof(server_bridge_buf));

		numRead = read(inotifyFd, buf, BUF_LEN);
		if (numRead <= 0) {
			perror(strerror(errno));
			printf("read() from inotify fd returned %d!", numRead);
			return 1;
		}

		for (p = buf; p < buf + numRead; ) {
			event = (struct inotify_event *) p;

			if((event->mask & IN_CLOSE_WRITE) && !strcmp(event->name, "client_message")){
				FILE *fp = fopen("/tmp/msg/client_message", "r");
				char ch;
				int count = 0;

				printf("Recv:");
				while((ch = fgetc(fp)) != EOF)
					client_bridge_buf[count++] = ch;
				client_bridge_buf[count] = '\0'
				system("rm -f /tmp/msg/client_message");


				printf("message read and send to server: %s\n",client_bridge_buf);
				strcpy(msg.mtext,client_bridge_buf);


				//using ipc to communicate with server
				//send message to server
				rc = msgsnd(msgqid, &msg, sizeof(msg.mtext), 0);
				if (rc < 0) {
					perror( strerror(errno) );
					printf("msgsnd failed, rc = %d\n", rc);
					return 1;
				}

				//message receive from server
				rc = msgrcv(msgqid2, &msg, sizeof(msg.mtext), 0, 0); 
				if (rc < 0) {
					perror( strerror(errno) );
					printf("msgrcv failed, rc=%d\n", rc);
					return 1;
				} 

				printf("Recv from server: %s\n",msg.mtext);
				strcpy(server_bridge_buf, msg.mtext);
				//finish the communication with server


				FILE *fp = fopen("/tmp/msg/bridge_message", "w");
				fputs(server_bridge_buf, fp);
				fclose(fp);
				break;

			}

			p += sizeof(struct inotify_event) + event->len;
		}
	}
}