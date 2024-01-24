#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "login.h"

node *head = NULL; // định nghĩa, không phải khai báo
node1 *head1 = NULL; // định nghĩa, không phải khai báo

void addNode(char* user, char* password, char *friend, char *offMsg){ // Them account vao ds lien ket
	node *temp = (struct Node*) malloc(sizeof(struct Node));
	strcpy(temp->username, user);
	strcpy(temp->pass, password);
	strcpy(temp->friend,friend);
	strcpy(temp->offMsg,offMsg);
	temp->state = 0; //trang thai online:1, off: 0
	temp->next = head;
	head = temp;
}

void addNodeRoom(char* name, char *cap, char *admin){ // Them account vao ds lien ket
	node1 *temp1 = (struct Node1*) malloc(sizeof(struct Node1));
	strcpy(temp1->name, name);
	strcpy(temp1->cap, cap);
	strcpy(temp1->admin, admin);
	//temp1->online = 0; //trang thai online:1, off: 0
	temp1->next=head1;
	head1 = temp1;
}

node *checkExist(char* user){ // xem ten account co trong danh sach lien ket chua??
	node *temp = head;
	if(strcmp(temp->username, user)==0){
		return temp;
	}
	while(temp->next != NULL){
		temp = temp->next;
		if(strcmp(temp->username, user)==0) return temp;
	}
	return NULL;
}

node1 *checkExistRoom(char* name){ // xem ten room co trong danh sach lien ket chua??
	node1 *temp1 = head1;
	if(strcmp(temp1->name, name)==0){
		return temp1;
	}
	while(temp1->next != NULL){
		temp1 = temp1->next;
		if(strcmp(temp1->name, name)==0) return temp1;
	}
	return NULL;
}

int checkPass(char* user, char* pass){ // check xem pass nhap dung chua??
	node *temp = head;
	while(temp != NULL){
		if(strcmp(temp->username, user) == 0){
			if(strcmp(temp->pass, pass) == 0) return 1;
			else return 0;
		}
		temp = temp->next;
	}
	return 0;
}

void writeFile(){ //ghi lai thong tin vao file
	node *temp = head;
	FILE *fp = fopen("user.txt", "w+");
	while(temp!=NULL){
		fprintf(fp, "%s %s %s %s", temp->username, temp->pass, temp->friend, temp->offMsg);
		temp = temp->next;
		if(temp!=NULL) fprintf(fp,"\n");
	}
	fclose(fp);
}

void writeRoomFile(int a){ //ghi lai thong tin vao file
	node1 *temp1 = head1;
	FILE *fp = fopen("groups.txt", "w+");
	fprintf(fp, "%d", a);
	fprintf(fp,"\n");
	int i;
	for(i=0;i<a;i++){
		fprintf(fp, "%s %s %s", temp1->name, temp1->cap,temp1->admin);
		temp1 = temp1->next;
		if(temp1!=NULL) fprintf(fp,"\n");
	}
	fclose(fp);
}

void printlist(){ // in ra ds lien ket
	node *temp = head;
	while(temp!=NULL){
		printf("%s %s\n", temp->username, temp->pass);
		temp = temp->next;
	}
}
