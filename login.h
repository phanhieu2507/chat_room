#ifndef __LOGIN_H__
#define __LOGIN_H__
#define BUFF_SIZE 8192

typedef struct Node{ //user node
	char username[30];
	char pass[30];
	int status;
	int state;
	int sock;
	int ID;
	char status2[30];
	char friend[200];
	char offMsg[300];
	struct Node *next;
} node;

typedef struct Node1{ //room node
	char name[30];
	char cap[10];
	int online;
	char admin[30];
	struct Node1 *next;
} node1;

extern node *head; // first node
extern node1 *head1;
void addNode(char* user, char* password, char* status2, int status, char* friend, char* offMsg);
void addNodeRoom(char* name, char *cap, char *admin);
node *checkExist(char* user);
node1 *checkExistRoom(char* name);
int checkPass(char* user, char* pass);
int checkStatus(char* user);
int lockAccount(char* user);
void writeFile();
void writeRoomFile(int a);
void printlist();
#endif