//mnt_echo_client.c
#include <sys/inotify.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>


#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

int main(int argc, char *argv[])
{
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

	//wahtch the change of file, fire a event if it changes then  getcwd get the path
	wd = inotify_add_watch(inotifyFd, "/tmp/msg/", IN_DELETE);
	if (wd == -1) {
		perror(strerror(errno));
		printf("inotify_add_watch\n");
		return 1;
	}



	msgqid = msgget(MAGIC, 0);
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

	while(1){

		// 1. send the message through writing to the client_message file
		// 2. client_message file will be read  and  deleted by bridge
		// 3. bridge will then send the message to server and receive the msg from server
		// 4. bridge will create the bridge_message file and write the message into it.
		// 5. after detecting the closing of the bridge_message file then open, read and delete the file 
		

		FILE *fp = fopen("/tmp/msg/client_message", "w");
		char input[4096], ch;
		while((ch = getchar()) != '\n'){
			fputc(ch, fp);
		}
		fputc('\n', fp);
		fclose(fp);



		numRead = read(inotifyFd, buf, BUF_LEN);
		if (numRead <= 0) {
			perror(strerror(errno));
			printf("read() from inotify fd returned %d!", numRead);
			return 1;
		}

		for (p = buf; p < buf + numRead; ) {
			event = (struct inotify_event *) p;
			if((event->mask|IN_CLOSE_WRITE) && !strcmp(event->name, "bridge_message")){

				FILE *fp = fopen("/tmp/msg/bridge_message", "r");
				char ch;

				printf("Recv:");
				while((ch = fgetc(fp)) != '\n')
					putchar(ch);
				printf("\n");
				system("rm -f /tmp/msg/bridge_message");

				break;

			}

			p += sizeof(struct inotify_event) + event->len;
		}

	}
}