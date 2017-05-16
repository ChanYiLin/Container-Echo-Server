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
	wd = inotify_add_watch(inotifyFd, "/tmp/msg/", IN_CLOSE_WRITE);
	if (wd == -1) {
		perror(strerror(errno));
		printf("inotify_add_watch\n");
		return 1;
	}

	FILE *fp_c = fopen("/tmp/msg/client_message", "w");
	char input[4096], ch;
	while((ch = getchar()) != '\n'){
		fputc(ch, fp_c);
		printf("receive char: %c\n",ch);
	}
	fputc('\n', fp_c);
	fclose(fp_c);

	while(1){

		// 1. send the message through writing to the client_message file
		// 2. client_message file will be read  and  deleted by bridge
		// 3. bridge will then send the message to server and receive the msg from server
		// 4. bridge will create the bridge_message file and write the message into it.
		// 5. after detecting the closing of the bridge_message file then open, read and delete the file 
		




		numRead = read(inotifyFd, buf, BUF_LEN);
		if (numRead <= 0) {
			perror(strerror(errno));
			printf("read() from inotify fd returned %d!", numRead);
			return 1;
		}

		for (p = buf; p < buf + numRead; ) {
			event = (struct inotify_event *) p;
			if((event->mask|IN_CLOSE_WRITE) && !strcmp(event->name, "bridge_message")){

				FILE *fp_b = fopen("/tmp/msg/bridge_message", "r");
				char ch;

				printf("Recv:");
				while((ch = fgetc(fp_b)) != '\n')
					putchar(ch);
				printf("\n");
				fclose(fp_b);
				system("rm -f /tmp/msg/bridge_message");


				break;

			}

			p += sizeof(struct inotify_event) + event->len;
		}

		
		FILE *fp_c = fopen("/tmp/msg/client_message", "w");
		char input[4096], ch;
		while((ch = getchar()) != '\n'){
			fputc(ch, fp_c);
			printf("receive char: %c\n",ch);
		}
		fputc('\n', fp_c);
		fclose(fp_c);

	}
}