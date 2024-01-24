#include <stdio.h>
void menu(){
	printf("\nUSER MANAGERMENT PROGRAM\n");
	printf("-----------------------------------\n");
	printf("1. Register\n");
	printf("2. Sign in\n");
	//printf("3. Sign out\n");
	printf("3. Exit\n");
	printf("Your choice (1-3):  ");
}


void chatFunction(){
	printf("\nCHATROOM FUNCTION\n");
	printf("-----------------------------------\n");
	printf("0. List member Online\n");
	printf("1. Create room\n");
	printf("2. Get in room\n");
	printf("3. List of rooms\n");
	// printf("4. Update status\n");
	printf("5. Chat 1vs1\n");
	printf("6. Sign Out\n");
	printf("7. List friend\n");
	printf("8. Add friend\n");
	printf("9. Un Friend\n");
	printf("10. Unread Message\n");
	printf("11. Send Offline Message\n");
	printf("Your choice (0-11):\n");
	//printf("a");
}

void menuhelp(){
	printf("/createroom : Create room\n");
	printf("/getroom : Get in room\n");
	printf("/listroom : List of rooms\n");
	printf("/listusergroup : List user of  rooms\n");
	printf("/touser : chat private to user\n");
	printf("/kickuser : kick user ( admin )\n");
	printf("/add: add user to room ( admin )\n");
}