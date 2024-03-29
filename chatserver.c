/* Chatserver */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdlib.h>
#include "common.h"
#include "login.h"
#include "logger.h"

#define MAXLEN 100

int max = 0;		   // so user
node *current[MAXLEN]; // nhung user da login
void readFile()
{
	FILE *fp = fopen("user.txt", "r");
	if (fp == NULL)
	{
		printf("Can't open the file!");
		exit(0);
	}
	char user[MAXLEN];
	char pass[MAXLEN];
	char friend[MAXLEN];
	char offMsg[500];
	while (feof(fp) == 0)
	{
		fscanf(fp, "%s %s %s %s\n", user, pass, friend, offMsg);
		addNode(user, pass, friend, offMsg);
		max++;
	}
}

void writeDataUser()
{
	node *temp = head;
	FILE *fp = fopen("user.txt", "w+");
	while (temp != NULL)
	{
		fprintf(fp, "%s %s %s %s", temp->username, temp->pass, temp->friend, temp->offMsg);
		temp = temp->next;
		if (temp != NULL)
			fprintf(fp, "\n");
	}
	fclose(fp);
}
/* Thông tin thành viên trong nhóm*/
typedef struct _member
{
	/* Tên thành viên */
	char *name;

	/* socket của thành viên */
	int sock;

	/* Phòng chat */
	int grid;

	/* Thành viên tiếp theo */
	struct _member *next;

	/* Thành viên trước đó */
	struct _member *prev;

} Member;

/*Thông tin phòng chat */
typedef struct _group11
{
	/* Tên phòng */
	int sock1;
	int sock2;
	char *name1;
	char *name2;
} Group11;
Group11 *chat11[MAXPKTLEN];
int sl = 1;

typedef struct _group
{
	/* Tên phòng */
	char *name;

	/* Số ng tối đa */
	int capa;

	/* Số người hiện tại */
	int occu;

	// admin
	char *admin;

	/* Dạnh sách liên két tất cả thành viên trong phòng */
	struct _member *mems;

} Group;

/* Các phòng chat */
Group group[1000];
int ngroups; // So groups

/*Tìm phòng theo tên */
int findgroup(char *name)
{
	int grid; /* ID phòng trò chuyện */

	for (grid = 0; grid < ngroups; grid++)
	{
		if (strcmp(group[grid].name, name) == 0)
			return (grid);
	}
	return (-1);
}
/* Tim tem user online*/
int findname(char *name)
{
	int m = 0;
	node *temp = head;
	while (temp != NULL)
	{
		if (strcmp(temp->username, name) == 0)
		{
			if (temp->state == 1)
				m = 1;
		}
		temp = temp->next;
	}
	return m;
}
// tim ten user
int findname2(char *name)
{
	int m = 0;
	node *temp = head;
	while (temp != NULL)
	{
		if (strcmp(temp->username, name) == 0)
		{
			m = 1;
		}
		temp = temp->next;
	}
	return m;
}

void removeSubstring(char *str, const char *sub)
{
	// Tạo một chuỗi tạm với dấu '/' và chuỗi con
	char temp[100];
	strcpy(temp, "/");
	strcat(temp, sub);

	// Nối chuỗi tạm vào chuỗi gốc
	strcat(str, temp);

	char *pos = strstr(str, temp);

	while (pos != NULL)
	{
		size_t len = strlen(temp);
		memmove(pos, pos + len, strlen(pos + len) + 1);
		pos = strstr(str, temp);
	}
}

/* ͨTìm thông tin thành viên trong nhom theo tên */
Member *findmemberbyname(char *name)
{
	int grid; /* ID phòng*/

	/*Vòng lặp tất cả các nhóm */
	for (grid = 0; grid < ngroups; grid++)
	{
		Member *memb;

		/* Duyệt tất cả thành viên cho mỗi phòng  */
		for (memb = group[grid].mems; memb; memb = memb->next)
		{
			if (strcmp(memb->name, name) == 0)
				return (memb);
		}
	}
	return (NULL);
}
// Tim nhom co user
int grid1(char *name)
{
	int grid; /* ID phòng*/

	/*Vòng lặp tất cả các nhóm */
	for (grid = 0; grid < ngroups; grid++)
	{
		Member *memb;

		/* Duyệt tất cả thành viên cho mỗi phòng  */
		for (memb = group[grid].mems; memb; memb = memb->next)
		{
			if (strcmp(memb->name, name) == 0)
				return (grid);
		}
	}
	return (NULL);
}
node *findsockbyname(char *username)
{
	node *temp = head;
	/* Duyệt tất cả các phòng */
	while (temp != NULL)
	{
		if (strcmp(temp->username, username) == 0)
		{
			return (temp);
		}
		temp = temp->next;
	}
	return (NULL);
}
/* ͨTìm thành viên thông qua socket */
Member *findmemberbysock(int sock)
{
	int grid; /*ID phòng*/

	/* Duyệt tất cả các phòng */
	for (grid = 0; grid < ngroups; grid++)
	{
		Member *memb;

		/* Duyệt tất cả các thành viên */
		for (memb = group[grid].mems; memb; memb = memb->next)
		{
			if (memb->sock == sock)
				return (memb);
		}
	}
	return (NULL);
}

/* Khởi tạo phòng chat */
int initgroups()
{
	FILE *fp;
	char name[MAXNAMELEN];
	char admin[MAXNAMELEN];
	char cap[BUFF_SIZE];
	int capa; // so luong toi da nguoi trong phong
	int grid; // id cua group

	/* Mở file lưu trữ thông tin chat */
	fp = fopen("groups.txt", "r");
	if (!fp)
	{
		fprintf(stderr, "error : unable to open file 'groups.txt'\n");
		return (0);
	}

	/* Số lượng phòng */
	fscanf(fp, "%d", &ngroups);

	/* Phân bổ bộ nhớ cho các phongf */
	if (!group)
	{
		printf("error : unable to calloc\n");
		return (0);
	}

	/*Thông tin phòng trò chuyeẹn từ file */
	for (grid = 0; grid < ngroups; grid++)
	{
		/* Tên phòng và số ng tối đa */
		if (fscanf(fp, "%s %d %s", name, &capa, admin) != 3)
		{
			printf("error : no info on group %d\n", grid + 1);
			return (0);
		}

		/* Lưu trữ thông tin vào cấu trúc nhóm */
		group[grid].name = strdup(name);
		group[grid].capa = capa;
		group[grid].occu = 0;
		group[grid].admin = strdup(admin);
		group[grid].mems = NULL;
		sprintf(cap, "%d", capa);
		addNodeRoom(name, cap, admin);
	}
	return (1);
}
int listUserGr(int sock)
{

	Member *memb;
	Member *sender;
	char bufrptr1[MAXPKTLEN];
	int t = 1;
	// char m[MAXLEN];
	node *temp;
	/* Nhận thông tin của thành viên qua socket */
	sender = findmemberbysock(sock);
	int id = grid1(sender->name);
	for (memb = group[id].mems; memb; memb = memb->next)
	{
		if (t == 1)
		{
			strcpy(bufrptr1, memb->name);
			t = 0;
		}
		else
		{
			strcat(bufrptr1, "/");
			strcat(bufrptr1, memb->name);
		}
	}
	/* Gui thong tin cac thanh vien co trong group ma socket hien tai dang tham gia */
	sendpkt(sock, LIST_USERGR, strlen(bufrptr1) + 1, bufrptr1);
	return (1);
}
/*dua ra danh sach nhung user online*/
int listOnline(int sock)
{
	char bufrptr1[MAXPKTLEN];
	int t = 1;
	char stt[10];
	node *temp = head;
	while (temp != NULL)
	{
		if (temp->state == 1) // user hien tai dang online
		{
			if (t == 1)
			{
				strcpy(bufrptr1, temp->username);
				t = 0;
			}
			else
			{
				strcat(bufrptr1, "/");
				strcat(bufrptr1, temp->username);
			}
		}
		temp = temp->next;
	}
	/* Gửi tin nhắn đến yêu cầu của khách hàng */
	sendpkt(sock, LISTUSERON, strlen(bufrptr1) + 1, bufrptr1);
	return (1);
}
node *findnamebysock(int sock)
{
	node *temp = head;
	/* Duyệt tất cả các phòng */
	while (temp != NULL)
	{
		if (temp->sock == sock)
		{
			return (temp);
		}
		temp = temp->next;
	}
	return (NULL);
}
int listFriend(int sock)
{
	char bufrptr1[MAXPKTLEN];
	node *temp = head;
	node *user;
	user = findnamebysock(sock);
	while (temp != NULL)
	{
		if (strcmp(temp->username, user->username) == 0) // user hien tai dang online
		{
			strcpy(bufrptr1, temp->friend);
			break;
		}
		temp = temp->next;
	}

	/* Gửi tin nhắn đến yêu cầu của khách hàng */
	sendpkt(sock, LIST_FRIENDS, strlen(bufrptr1) + 1, bufrptr1);
	return (1);
}
int listOfflineMsg(int sock)
{
	char bufrptr1[MAXPKTLEN];
	node *temp = head;
	node *user;
	user = findnamebysock(sock);
	while (temp != NULL)
	{
		if (strcmp(temp->username, user->username) == 0) // tim user trong node
		{
			strcpy(bufrptr1, temp->offMsg);
			break;
		}
		temp = temp->next;
	}

	/* Gửi tin nhắn đến yêu cầu của khách hàng */
	sendpkt(sock, LIST_OFF_MESSAGES, strlen(bufrptr1) + 1, bufrptr1);
	return (1);
}

/* Gửi tất cả thông tin phòng trò chuyện cho khách hàng */
int listgroups(int sock)
{
	int grid, i;
	char bufrptr1[MAXPKTLEN], length[MAXPKTLEN];
	/* Mỗi phần thông tin được phân tách bằng NULL trong chuỗi */
	sprintf(length, "%d", ngroups);
	strcpy(bufrptr1, length);
	for (grid = 0; grid < ngroups; grid++)
	{
		{
			strcat(bufrptr1, "/");
			strcat(bufrptr1, group[grid].name);
			strcat(bufrptr1, "/");
			char str[100];
			sprintf(str, "%d", group[grid].capa);
			strcat(bufrptr1, str);
			strcat(bufrptr1, "/");
			sprintf(str, "%d", group[grid].occu);
			strcat(bufrptr1, str);
		}
	}
	/* Gửi tin nhắn đến yêu cầu của khách hàng */
	sendpkt(sock, LIST_GROUPS, strlen(bufrptr1) + 1, bufrptr1);
	return (1);
}

int processLogIn(int sock, char *username, char *pass)
{
	// check username
	if (findname(username))
	{
		char *errmsg = "-> account is login!\n";
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg);
		return 0;
	}
	if (checkExist(username) == NULL)
	{
		char *errmsg = "-> Cannot find account!\n";
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg);
		return 0;
	}
	if (checkPass(username, pass) == 0)
	{
		char *errmsg = "Password is incorrect!\n";
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg);
		return 0;
	}
	char *succmsg = "Log in successful!\n";
	current[sock] = checkExist(username); // return node user dang nhap vao
	current[sock]->state = 1;			  // user dang online
	current[sock]->sock = sock;
	sendpkt(sock, SUCCESS, strlen(succmsg), succmsg);
	printf("%d\n", sock);
	return 1;
}

int processRegister(int sock, char *username, char *pass)
{
	if (current[sock] != NULL)
	{
		char *errmsg = "->You are currently logged in. Please log out to register.\n";
		char logMessage[256];
		snprintf(logMessage, sizeof(logMessage), "Error: In socket %d, there is 1 account currently logged in. Please log out to be able to register a new account.", sock);
		writeToLog(logMessage);
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg);
		return 0;
	}
	if (checkExist(username) != NULL)
	{
		char *errmsg = "-> Account existed!\n";
		char logMessage[256];
		snprintf(logMessage, sizeof(logMessage), "Error: Account existed");
		writeToLog(logMessage);
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg);
		return 0;
	}
	addNode(username, pass, "NULL", "NULL");
	writeDataUser();
	max++;
	char logMessage[256];
	snprintf(logMessage, sizeof(logMessage), "Register successful.");
	writeToLog(logMessage);
	char *succmsg = "Register successful!\n";
	sendpkt(sock, SUCCESS, strlen(succmsg), succmsg);
	return 1;
}

int processCreatRoom(int sock, char *name, char *cap)
{
	if (checkExistRoom(name) != NULL)
	{
		char *errmsg = "-> Room existed!\n";
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg);
		return 0;
	}
	addNodeRoom(name, cap, current[sock]->username);

	writeRoomFile(ngroups + 1);
	// printf("%d\n",ngroups);
	ngroups++;
	// group = realloc(group, 1 * sizeof(int));
	group[ngroups - 1].name = strdup(name);
	group[ngroups - 1].capa = atoi(cap);
	group[ngroups - 1].occu = 0;
	group[ngroups - 1].admin = strdup(current[sock]->username);
	group[ngroups - 1].mems = NULL;

	// printf("%s\n",group[ngroups-1].name);
	char *succmsg = "Create successful!\n";
	sendpkt(sock, SUCCESS, strlen(succmsg), succmsg);
	return 1;
}


int processLogout(int sock, char *username)
{
	if (current[sock] == NULL)
	{
		char *errmsg = "-> You are not loged in!\n";
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg);
		return 0;
	}
	if (checkExist(username) == NULL)
	{
		char *errmsg = "-> Cannot find account!\n";
		char logMessage[256];
		snprintf(logMessage, sizeof(logMessage), "Error:Cannot find account.");
		writeToLog(logMessage);
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg);
		return 0;
	}
	if (strcmp(current[sock]->username, username) != 0)
	{
		char *errmsg = "-> Account is not sign in!\n";
		char logMessage[256];
		snprintf(logMessage, sizeof(logMessage), "Error: Account is not sign in!");
		writeToLog(logMessage);
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg);
		return 0;
	}
	current[sock]->state = 0;
	current[sock] = NULL;

	char *succmsg = "Log out successful!\n";
	char logMessage[256];
	snprintf(logMessage, sizeof(logMessage), "Log out successful!");
	writeToLog(logMessage);
	sendpkt(sock, SUCCESS, strlen(succmsg), succmsg);

	return 1;
}

/* Tham gia phòng chat */
int joingroup(int sock, char *gname, char *username)
{
	int grid;
	Member *memb;

	/*  Nhận ID phòng trò chuyện dựa trên tên phòng trò chuyện  */
	grid = findgroup(gname);
	if (grid == -1)
	{
		char *errmsg = "This group doesn't exist!";
		char logMessage[256];
		snprintf(logMessage, sizeof(logMessage), "Error: This group chat doesn't exist!");
		writeToLog(logMessage);
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg);
		return (0);
	}

	leavegroup(sock);

	memb = findmemberbyname(username);

	/* Nếu tên thành viên trò chuyện đã tồn tại, trả về thông báo lỗi */
	if (group[grid].capa == group[grid].occu)
	{
		char *errmsg = "room is full";
		char logMessage[256];
		snprintf(logMessage, sizeof(logMessage), "Error:This group chat is full.");
		writeToLog(logMessage);
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg); /* gửi tin nhắn tham gia từ chối*/
		return (0);
	}

	/*Kiểm tra xem phòng trò chuyện đã đầy chưa*/
	memb = (Member *)calloc(1, sizeof(Member));
	if (!memb)
	{
		printf("error : unable to calloc\n");
	}
	memb->name = strdup(username);
	printf("%s , %s\n", memb->name, username);
	memb->sock = sock;
	memb->grid = grid;
	memb->prev = NULL;
	memb->next = group[grid].mems;
	if (group[grid].mems)
	{
		group[grid].mems->prev = memb;
	}
	group[grid].mems = memb;
	printf("admin: '%s' joined '%s'\n", username, gname);
	char logMessage[256];
	snprintf(logMessage, sizeof(logMessage), "'%s' joined '%s'\n", username, gname);
	writeToLog(logMessage);
	changeStatus1(sock);
	/* Cập nhật phòng chat trực tuyến */
	group[grid].occu++;
	printf("%d\n", current[sock]->sock);
	sendpkt(sock, JOIN_ACCEPTED, 0, NULL); /* Gửi và nhận tin nhắn thành viên */
	fflush(stdin);
	return (1);
}
// tim sock id bang username
int try(char *username)
{
	node *temp = head;
	while (1)
	{
		if (strcmp(temp->username, username) == 0)
			break;
		else
			temp = temp->next;
	}
	return temp->sock;
}

void try1(char *a)
{
	node *temp = head;
	while (temp != NULL)
	{
		if (strcmp(temp->username, a) == 0)
		{
			temp->ID = sl;
			temp->state = 0;
		}
		temp = temp->next;
	}
	
}

int findbysock(int sock)
{
	int m;
	node *temp = head;
	while (temp != NULL)
	{
		if (temp->sock == sock)
		{
			m = temp->ID;
		}
		temp = temp->next;
	}
	return m;
}

int findother(int sock)
{
	printf("input other sock %d\n", sock);
	int m;
	printf("head here %d - %d\n", head->ID, head->sock);
	node *temp = head;

	while (temp != NULL)
	{
		if (temp->ID == findbysock(sock) && temp->sock != sock)
		{
			m = temp->sock;
		}
		temp = temp->next;
	}
	return m;
}

int changeStatus(char *name)
{
	node *temp = head;
	while (temp != NULL)
	{
		if (strcmp(temp->username, name) == 0)
		{
			temp->state = 0;
		}
		temp = temp->next;
	}
}
// join chat 11, uname la user muon chat
int join11(int sock, char *uname, char *username)
{
	int m = 0, n;
	node *cur1, *cur2;
	/* Không thể tự chat với bản thân */
	if (strcmp(current[sock]->username, uname) == 0)
	{
		char *errmsg = "Can't talk with yourself";
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg); /* gửi tin nhắn từ chối tham gia */
		return (0);
	}
	if (!findname(uname))
	{
		char *errmsg = "This user isn't available!";
		sendpkt(sock, JOIN_REJECTED, strlen(errmsg), errmsg);
		return (0);
	}
	printf("start new chat1v1 %s - %s\n", username, uname);

	try1(username); // 2 user dang chat voi nhau
	try1(uname);	// se co cung id, dat trang thai la offline
	sl++;
	printf("send initial pkt to %s from %s - len %zd\n", uname, current[sock]->username, strlen(current[sock]->username));
	sendpkt(try(uname), REQUEST, strlen(current[sock]->username) + 1, current[sock]->username);
	printf("send join accepted pkt to %s\n", uname);
	return (1);
}
// add friend uname la user muon chat
int requestAddFriend(int sock, char *uname, char *username)
{
	int m = 0, n;
	node *cur1, *cur2;
	/* Không thể tự ket ban với bản thân */
	if (strcmp(current[sock]->username, uname) == 0)
	{
		char *errmsg = "Can't add friend to my self";
		sendpkt(sock, FRIEND_REJECT, strlen(errmsg), errmsg); /* gửi tin nhắn từ chối tham gia */
		return (0);
	}
	if (!findname(uname))
	{
		char *errmsg = "This user isn't available!";
		sendpkt(sock, FRIEND_REJECT, strlen(errmsg), errmsg);
		return (0);
	}
	printf("add friend %s - %s\n", username, uname);
	sendpkt(try(uname), REQUEST_ADD_FRIEND, strlen(current[sock]->username) + 1, current[sock]->username);

	return (1);
}
int requestUnFriend(int sock, char *uname, char *username)
{
	/* Không thể tự huy ket ban với bản thân */
	if (strcmp(current[sock]->username, uname) == 0)
	{
		char *errmsg = "Can't unfriend to my self";
		sendpkt(sock, FAILED, strlen(errmsg), errmsg); /* gửi tin nhắn từ chối tham gia */
		return (0);
	}
	if (!findname2(uname))
	{
		char *errmsg = "This user isn't available!";
		sendpkt(sock, FAILED, strlen(errmsg), errmsg);
		return (0);
	}
	printf("unfriend %s - %s\n", username, uname);

	node *temp = head;
	while (temp != NULL)
	{
		if (strcmp(temp->username, uname) == 0 || strcmp(temp->username, username) == 0)
		{
			if (strcmp(temp->username, uname) == 0)
			{
				removeSubstring(temp->friend, username);
			}
			else
				removeSubstring(temp->friend, uname);
		}
		temp = temp->next;
	}
	writeDataUser();
	sendpkt(sock, SUCCESS, strlen(current[sock]->username) + 1, current[sock]->username);

	return (1);
}

int requestSendOffMessage(int sock, char *uname, char *username, char *msg)
{
	/* Không thể tự huy ket ban với bản thân */
	if (strcmp(current[sock]->username, uname) == 0)
	{
		char *errmsg = "Can't send message to my self";
		sendpkt(sock, FAILED, strlen(errmsg), errmsg); /* gửi tin nhắn từ chối tham gia */
		return (0);
	}
	if (!findname2(uname))
	{
		char *errmsg = "This user isn't available!";
		sendpkt(sock, FAILED, strlen(errmsg), errmsg);
		return (0);
	}
	printf("send message offline %s - %s: %s\n", username, uname, msg);

	node *temp = head;
	while (temp != NULL)
	{
		if (strcmp(temp->username, uname) == 0)
		{
			strcat(temp->offMsg, "/");
			strcat(temp->offMsg, username);
			strcat(temp->offMsg, ":");
			strcat(temp->offMsg, msg);
		}
		temp = temp->next;
	}
	writeDataUser();
	sendpkt(sock, SUCCESS, strlen(current[sock]->username) + 1, current[sock]->username);

	return (1);
}

int changeStatus1(int sock)
{
	node *temp = head;
	while (temp != NULL)
	{
		if (temp->sock == sock)
			temp->state = 1;
		temp = temp->next;
	}
}

int leave11(int sock)
{
	changeStatus1(sock);
	changeStatus1(findother(sock));
	sendpkt(findother(sock), QUIT, 0, NULL);
}

/* Rời khỏi phòng */
int leavegroup(int sock)
{
	Member *memb;
	node *temp;
	/* Nhận thông tin thành viên phòng chat */
	temp = findnamebysock(sock);
	memb = findmemberbysock(sock);
	if (!memb)
		return (0);

	/*Xóa thành viên */
	if (memb->next)
		memb->next->prev = memb->prev; /* Cuối ds thành viên phòng chat*/

	/* remove from ... */
	if (group[memb->grid].mems == memb) /*Đầu danh sách liên kết của các thành viên phòng chat */
		group[memb->grid].mems = memb->next;

	else
		memb->prev->next = memb->next; /*Ở giữa ds*/

	printf("admin: '%s' left '%s'\n",
		   temp->username, group[memb->grid].name);

	/*Cập nhật chia sẻ phòng chat*/
	group[memb->grid].occu--;
	temp->state = 1;
	/* Giải phóng bộ nhớ*/
	// free(memb->sock);
	free(memb);
	return (1);
}

char *name(int sock)
{
	char *name;
	node *temp = head;
	while (temp != NULL)
	{
		if (temp->sock == sock)
		{
			strcpy(name, temp->username);
		}
		temp = temp->next;
	}
	return name;
}
// sock: sock id cua nguoi dung hien tai, text: ten nguoi muon kick
int kickuser(int sock, char *text)
{
	Member *memb;
	Member *sender;
	char *admin;
	node *temp;
	/* Nhận thông tin của thành viên qua socket */
	sender = findmemberbysock(sock);
	temp = findnamebysock(sock);
	if (!sender)
	{
		printf("strange: no member at %d\n", sock);
		return (0);
	}
	admin = group[sender->grid].admin;
	if (strcmp(admin, current[sock]->username) != 0)
	{
		char *errmsg = "you are not admin";
		sendpkt(sock, KICK, strlen(errmsg) + 1, errmsg);
	}
	else
	{
		int d = 0;
		for (memb = group[sender->grid].mems; memb; memb = memb->next)
		{
			/*Bỏ qua người gửi */
			if (memb->sock == sock)
			{
				continue;
			}
			if (strncmp(memb->name, text, strlen(text)) == 0)
			{
				d = 1;
				sendpkt(memb->sock, KICKU, 0, NULL);
			}
		}
		if (d == 1)
		{
			char *errmsg = "kick success";
			sendpkt(sock, KICK, strlen(errmsg) + 1, errmsg);
		}
		else
		{
			char *errmsg = "user is not in this room";
			sendpkt(sock, KICK, strlen(errmsg) + 1, errmsg);
		}
	}
	return (1);
}
int sendApcept(int sock, char *text)
{
	char *tl, *name;
	tl = strtok(text, "/");
	name = strtok(NULL, "/");
	if (strcmp(tl, "y") == 0)
	{
		sendpkt(sock, SUCCESS, 0, NULL);
		sendpkt(try(name), JOIN_ACCEPTED, 0, NULL);
	}
	else
	{
		// sendpkt(sock, SUCCESS, 0, NULL);
		// sendpkt(try(name),JOIN_ACCEPTED,0,NULL);
		sendpkt(sock, DONE, 0, NULL);
		sendpkt(try(name), DONE, 0, NULL);
	}
}

int processAddFriend(int sock, char *text)
{
	char *tl, *name;
	tl = strtok(text, "/");
	name = strtok(NULL, "/");
	if (strcmp(tl, "y") == 0)
	{
		node *temp = findnamebysock(sock);
		strcat(temp->friend, "/");
		strcat(temp->friend, name);
		node *temp2 = head;

		while (temp2 != NULL)
		{
			if (strcmp(temp2->username, name) == 0)
			{
				strcat(temp2->friend, "/");
				strcat(temp2->friend, temp->username);
			}
			temp2 = temp2->next;
		}
		writeDataUser();
		sendpkt(sock, FRIEND_ACCEPT, 0, NULL);
		sendpkt(try(name), FRIEND_ACCEPT, 0, NULL);
	}
	else
	{
		sendpkt(sock, FRIEND_REJECT, 0, NULL);
		sendpkt(try(name), FRIEND_REJECT, 0, NULL);
	}
}

int processAddMember(int sock, char *text)
{
	char *tl, *name;
	tl = strtok(text, "/");
	name = strtok(NULL, "/");
	int grid;
	Member *memb;
	Member *admin;
	node *temp;
	if (strcmp(tl, "y") == 0)
	{
		temp = findnamebysock(sock);
		admin = findmemberbysock(try(name));
		grid = admin->grid;
		leavegroup(sock);
		memb = (Member *)calloc(1, sizeof(Member));
		memb->name = strdup(temp->username);
		memb->sock = sock;
		memb->grid = grid;
		memb->prev = NULL;
		memb->next = group[grid].mems;
		if (group[grid].mems)
		{
			group[grid].mems->prev = memb;
		}
		group[grid].mems = memb;
		printf("Admin: '%s' joined '%s'\n", temp->username, group[grid].name);
		changeStatus1(sock);
		group[grid].occu++;
		printf("%d\n", current[sock]->sock);
		sendpkt(sock, MEMBER_ACCEPT, 0, NULL);
		sendpkt(try(name), MEMBER_ACCEPT, 0, NULL);
		fflush(stdin);
	}
	else
	{
		sendpkt(sock, MEMBER_REJECT, 0, NULL);
		sendpkt(try(name), MEMBER_REJECT, 0, NULL);
	}
}
int givemsg(int sock, char *text)
{
	char pktbufr[MAXPKTLEN];
	char *bufrptr;
	long bufrlen;
	int tnt;
	node *temp = head;
	tnt = findbysock(sock);
	/* Thêm tên người gửi trc văn bản tin nhắn */
	bufrptr = pktbufr;
	strcpy(bufrptr, text);
	bufrptr += strlen(bufrptr) + 1;
	bufrlen = bufrptr - pktbufr;
	printf("other sock %d\n", findother(sock));
	sendpkt(findother(sock), USER_TEXT1, bufrlen, pktbufr);
	printf("%s", pktbufr);
	/* Truyên tin nhắn đến các thành viên khác trong phòng*/
	/*Bỏ qua người gửi */
	fflush(stdin);
	printf("%s", text);
	return (1);
}
int toUser(int sock, char *text)
{
	Member *memb;
	Member *sender;
	char pktbufr[MAXPKTLEN];
	char *bufrptr, bufrptr1[MAXPKTLEN], *name, *content;
	long bufrlen;

	node *temp;
	/* Nhận thông tin của thành viên qua socket */
	sender = findmemberbysock(sock);
	temp = findnamebysock(sock);
	if (!sender)
	{
		printf("strange: no member at %d\n", sock);
		return (0);
	}
	// temp = findnamebysock(sock);
	/* Thêm tên người gửi trc văn bản tin nhắn */
	bufrptr = pktbufr;
	strcpy(bufrptr, temp->username);
	strcpy(bufrptr1, temp->username);
	// bufrptr1 = strdup(temp->username);
	strcat(bufrptr1, "/");

	name = strtok(text, "/");
	content = strtok(NULL, "/");
	// name[strlen(name) - 1] = '\0';
	// char m[MAXLEN];
	strcat(bufrptr1, content);
	printf("%s\n", bufrptr1);
	bufrptr += strlen(bufrptr) + 1;
	strcpy(bufrptr, text);
	bufrptr += strlen(bufrptr) + 1;
	bufrlen = bufrptr - pktbufr;
	/* Truyên tin nhắn đến các thành viên khác trong phòng*/
	for (memb = group[sender->grid].mems; memb; memb = memb->next)
	{
		/*Bỏ qua người gửi */
		if (memb->sock == sock)
		{
			continue;
		}
		if (strncmp(memb->name, name, strlen(name) - 1) == 0)
		{
			sendpkt(memb->sock, USER_TEXT, strlen(bufrptr1) + 1, bufrptr1); /* Gửi tin nhắn cho các thành viên khác trong phòng trò chuyện (TCP là song công hoàn toàn) */
		}
	}
	// printf("%d\n", sender->sock);
	printf("%s: %s", temp->username, text);
	return (1);
}

// sock: id cua nguoi dung hien tai, text: ten nguoi muon them
int addMember(int sock, char *text)
{
	Member *memb;
	Member *sender;
	char *admin;
	node *temp;

	sender = findmemberbysock(sock);
	temp = findnamebysock(sock);
	if (!sender)
	{
		printf("strange: no member at %d\n", sock);
		return (0);
	}
	admin = group[sender->grid].admin;

	if (strcmp(admin, current[sock]->username) != 0)
	{
		char *errmsg = "You are not admin";
		// char logMessage[256];
		// snprintf(logMessage, sizeof(logMessage), "Error: '%s' at socket %d isn't group chat 's admin.", current[sock]->username, sock);
		// writeToLog(logMessage);
		sendpkt(sock, ADD_MEMBER, strlen(errmsg) + 1, errmsg);
	}
	else
	{
		int d = 0;
		for (memb = group[sender->grid].mems; memb; memb = memb->next)
		{
			if (memb->sock == sock)
				continue;
			if (strncmp(memb->name, text, strlen(text)) == 0)
			{
				d++;
			}
		}
		if (d == 0)
		{
			sendpkt(try(text), ASK_MEMBER, strlen(current[sock]->username) + 1, current[sock]->username);
			// char logMessage[256];
			// snprintf(logMessage, sizeof(logMessage), "'%s' at socket %d sends a request to join group chat to '%s' at socket '%d'", current[sock]->username, sock, text, try(text));
			// writeToLog(logMessage);
		}
		else
		{
			char *msg = "User is already in the group chat.";
			// char logMessage[256];
			// snprintf(logMessage, sizeof(logMessage), "Error: '%s' at socket %d is already in the group chat.", text, try(text));
			// writeToLog(logMessage);
			sendpkt(sock, ADD_MEMBER, strlen(msg) + 1, msg);
		}
	}
}
/* Gửi tin nhắn đên các thành viên khác trong phòng chat */
int relaymsg(int sock, char *text)
{
	Member *memb;
	Member *sender;
	char pktbufr[MAXPKTLEN];
	char *bufrptr, bufrptr1[MAXPKTLEN];
	long bufrlen;
	// char m[MAXLEN];
	node *temp;
	/* Nhận thông tin của thành viên qua socket */
	sender = findmemberbysock(sock);
	temp = findnamebysock(sock);
	if (!sender)
	{
		printf("strange: no member at %d\n", sock);
		return (0);
	}
	// temp = findnamebysock(sock);
	/* Thêm tên người gửi trc văn bản tin nhắn */
	bufrptr = pktbufr;
	strcpy(bufrptr, temp->username);
	strcpy(bufrptr1, temp->username);
	// bufrptr1 = strdup(temp->username);
	strcat(bufrptr1, "/");
	strcat(bufrptr1, text);
	printf("%s\n", bufrptr1);
	bufrptr += strlen(bufrptr) + 1;
	strcpy(bufrptr, text);
	bufrptr += strlen(bufrptr) + 1;
	bufrlen = bufrptr - pktbufr;
	/* Truyên tin nhắn đến các thành viên khác trong phòng*/
	for (memb = group[sender->grid].mems; memb; memb = memb->next)
	{
		/*Bỏ qua người gửi */
		if (memb->sock == sock)
		{
			continue;
		}
		sendpkt(memb->sock, USER_TEXT, bufrlen, bufrptr1); /* Gửi tin nhắn cho các thành viên khác trong phòng trò chuyện (TCP là song công hoàn toàn) */
	}
	// printf("%d\n", sender->sock);
	printf("%s: %s", temp->username, text);
	return (1);
}

int repmenu(int sock, char *text)
{
	sendpkt(sock, MENU, strlen(text), text);
	return (1);
}

/*main*/
int main(int argc, char *argv[])
{
	int servsock;			   /* Mô tả socket máy chủ */
	int maxsd;				   /* Số máy tối đa cho phép kết nối đến socketֵ */
	fd_set livesdset, tempset; /* Bộ thăm dò socket*/
	readFile();
	/*Check cú pháp */
	if (argc != 2)
	{
		printf("Wrong syntax!!!\n--> Correct Syntax: ./chatserver PortNumber\n");
		return 0;
	}

	/* Khởi tạo thông tin phòng */
	if (!initgroups())
		exit(1);

	/* Chức năng xử lí tín hiệu */
	// signal(SIGTERM, cleanup);
	// signal(SIGINT, cleanup);

	/* Sẵn sàng nhận yêu cầu */
	servsock = startserver(argv[1]); /*Đc xác định trong chatlinker.c, Hoàn thành socket, port, và chuyển socket sag listen */
	if (servsock == -1)
		exit(1);

	/* Khởi tạo maxsd */
	maxsd = servsock;

	/* Khởi tạo bộ thăm dò */
	FD_ZERO(&livesdset);		  /* Khởi tạo bộ thăm dò livesdset*/
	FD_ZERO(&tempset);			  /* Khởi tạo bộ thăm dò tempset */
	FD_SET(servsock, &livesdset); /*Thêm servesock vào livesdset*/

	/* Xử lý yêu cầu */
	while (1)
	{
		int sock; /* vòng lặp */

		tempset = livesdset;

		/* Yêu cầu liên kết tới socket ms*/
		select(maxsd + 1, &tempset, NULL, NULL, NULL);

		/* vòng lặp */
		for (sock = 3; sock <= maxsd; sock++)
		{
			/* Nếu máy chủ lắng nghe phai socket cua may chu, nó sẽ nhảy ra khỏi gói nhận và thực hiện kết nối chấp nhận */
			if (sock == servsock)
				continue;

			/* Có một tin nhắn từ socket máy khách */
			if (FD_ISSET(sock, &tempset))
			{
				Packet *pkt;
				/* Đọc tin nhắn */
				pkt = recvpkt(sock); /* Hàm recvpkt được định nghĩa trong "chatlinker.c" */

				if (!pkt)
				{
					/* Máy khách bị ngắt kết nố */
					char *clientname; /* Tên máy khách */

					/* Sử dụng hàm gethostbyaddr, getpeername để lấy tên máy khách */
					socklen_t len;
					struct sockaddr_in addr;
					len = sizeof(addr);
					if (getpeername(sock, (struct sockaddr *)&addr, &len) == 0)
					{
						struct sockaddr_in *s = (struct sockaddr_in *)&addr;
						struct hostent *he;
						he = gethostbyaddr(&s->sin_addr, sizeof(struct in_addr), AF_INET);
						clientname = he->h_name;
					}
					else
						printf("Cannot get peer name/n");

					printf("admin: disconnect from '%s' at '%d'\n",
						   clientname, sock);

					/* Xóa thành viên khỏi phòng trò chuyện */
					leavegroup(sock);

					close(sock);

					/* Xóa sock khỏi bộ thăm dò livesdset */
					FD_CLR(sock, &livesdset);
				}
				else
				{
					// CHEN LOGIN VAO DAY
					char *gname, *mname, *username, *pass, *name, *uname, *msg;
					char *cap;
					char logMessage[256];
					/* loại hành động */

					switch (pkt->type)
					{

					case REGISTER:
						username = strtok(pkt->text, "/");
						pass = strtok(NULL, "/");
						snprintf(logMessage, sizeof(logMessage), "User at socket %d requested registration.", sock);
						writeToLog(logMessage);
						processRegister(sock, username, pass);
						break;
					case CREAT_ROOM:
						name = strtok(pkt->text, "/");
						cap = strtok(NULL, "/");
						snprintf(logMessage, sizeof(logMessage), "User at socket %d requested to create a group chat.", sock);
						writeToLog(logMessage);
						processCreatRoom(sock, name, cap);
						break;
					case LOG_IN:
						username = strtok(pkt->text, "/");
						pass = strtok(NULL, "/");
						snprintf(logMessage, sizeof(logMessage), "User at socket %d requested to login with username '%s'.", sock, username);
						writeToLog(logMessage);
						processLogIn(sock, username, pass);
						break;
					case LOG_OUT:
						username = pkt->text;
						snprintf(logMessage, sizeof(logMessage), "'%s' at socket %d requested to logout", username, sock);
						writeToLog(logMessage);
						processLogout(sock, username);
						break;
					case REQ_CHAT_11:
						username = pkt->text;
						join11(sock, username, current[sock]->username);
						break;
					case ADD_FRIEND:
						username = pkt->text;
						snprintf(logMessage, sizeof(logMessage), "'%s' at socket %d requested to be friend with '%s' at socket %d.", current[sock]->username, sock, username, try(username));
						writeToLog(logMessage);
						requestAddFriend(sock, username, current[sock]->username);
						break;
					case UN_FRIEND:
						username = pkt->text;
						snprintf(logMessage, sizeof(logMessage), "'%s' at socket %d requested to unfriend with '%s' at socket %d.", current[sock]->username, sock, username, try(username));
						writeToLog(logMessage);
						requestUnFriend(sock, username, current[sock]->username);
						break;
					case SEND_OFF_MESSAGES:
						username = strtok(pkt->text, "/");
						msg = strtok(NULL, "/");
						snprintf(logMessage, sizeof(logMessage), "'%s' at socket %d requested to send an offline message to '%s'.", current[sock]->username, sock, username);
						writeToLog(logMessage);
						requestSendOffMessage(sock, username, current[sock]->username, msg);
						break;
					case REP_ADD_FRIEND:
						processAddFriend(sock, pkt->text);
						break;
					case MEMBER_ANSWER:
						processAddMember(sock, pkt->text);
						break;
					case LIST_GROUPS:
						listgroups(sock);
						break;
					case JOIN_GROUP:
						gname = pkt->text;
						snprintf(logMessage, sizeof(logMessage), "'%s' at socket %d requested to join group chat named '%s'", current[sock]->username, sock, gname);
						writeToLog(logMessage);
						joingroup(sock, gname, current[sock]->username);
						break;
					case LISTUSERON:
						listOnline(sock);
						snprintf(logMessage, sizeof(logMessage), "'%s' at socket %d requested to show list of users online", current[sock]->username, sock);
						writeToLog(logMessage);
						break;
					case LIST_FRIENDS:
						listFriend(sock);
						snprintf(logMessage, sizeof(logMessage), "'%s' at socket %d requested to show list of friends", current[sock]->username, sock);
						writeToLog(logMessage);
						break;
					case LIST_OFF_MESSAGES:
						listOfflineMsg(sock);
						snprintf(logMessage, sizeof(logMessage), "'%s' at socket %d requested to show list of offline messages", current[sock]->username, sock);
						writeToLog(logMessage);
						break;
					case LIST_USERGR:
						listUserGr(sock);
						snprintf(logMessage, sizeof(logMessage), "'%s' at socket %d requested to show list of users in the group chat", current[sock]->username, sock);
						writeToLog(logMessage);
						break;
					case LEAVE_GROUP:
						leavegroup(sock);
						snprintf(logMessage, sizeof(logMessage), "'%s' at socket %d requested to left the current group chat", current[sock]->username, sock);
						writeToLog(logMessage);
						break;
					case TO:
						toUser(sock, pkt->text);
						break;
					case ADD_MEMBER:
						addMember(sock, pkt->text);
						snprintf(logMessage, sizeof(logMessage), "'%s' at socket %d requested to add '%s' at socket %d to group chat.", current[sock]->username, sock, pkt->text, try(pkt->text));
						writeToLog(logMessage);
						break;
					case USER_TEXT:
						relaymsg(sock, pkt->text);
						Member *sender;
						sender = findmemberbysock(sock);
						int grid = sender->grid;
						snprintf(logMessage, sizeof(logMessage), "'%s' at socket %d sent a message to the group chat named '%s'.", current[sock]->username, sock, group[grid].name);
						writeToLog(logMessage);
						break;
					case MENU:
						repmenu(sock, pkt->text);
						break;
					case USER_TEXT1:
						givemsg(sock, pkt->text);
						break;
					case QUIT:
						leave11(sock);
						break;
					case REP_CHAT_11:
						sendApcept(sock, pkt->text);
						break;
					case KICK:
						kickuser(sock, pkt->text);
						break;
					}
					/*Cấu trúc gói phát hành */
					freepkt(pkt);
				}
			}
		}

		struct sockaddr_in remoteaddr; /* cấu trúc địa chỉ máy khách */
		socklen_t addrlen;

		/* CÓ một y/c từ 1 user mới */
		if (FD_ISSET(servsock, &tempset))
		{
			int csd; /*mô tả socket được kết nối */

			/* Chấp nhận yêu cầu kết nối mới */
			addrlen = sizeof remoteaddr;
			csd = accept(servsock, (struct sockaddr *)&remoteaddr, &addrlen);

			/* Kết nối thành công */
			if (csd != -1)
			{
				char *clientname;

				/* Nhận tên máy chủ của khách hàng bằng cách sử dụng chức năng gethostbyaddr*/
				struct hostent *h;
				h = gethostbyaddr((char *)&remoteaddr.sin_addr.s_addr,
								  sizeof(struct in_addr), AF_INET);

				if (h != (struct hostent *)0)
					clientname = h->h_name;
				else
					printf("gethostbyaddr failed\n");

				/* Hiển thị tên máy chủ của máy khách và bộ mô tả ổ cắm tương ứng  */
				printf("admin: connect from '%s' at '%d'\n",
					   clientname, csd);

				char logMessage[256];
				snprintf(logMessage, sizeof(logMessage), "New connection from '%s' at socket '%d'", clientname, csd);
				writeToLog(logMessage);

				/*Thêm csd vào livesdset */
				FD_SET(csd, &livesdset);

				/*maxsd: Socket lớn nhất */
				if (csd > maxsd)
					maxsd = csd;
			}
			else
			{
				perror("accept");
				exit(0);
			}
		}
	}
}
