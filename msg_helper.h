#define MSGPERM 0600    
#define MSGTXTLEN 128 
#define MAGIC  6000
#define MAGIC2 6001

struct msg_buf {
	long mtype;
	char mtext[MSGTXTLEN];
};

