#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LEN 50
#define MAX_LINE_LEN 100
#define MAX_FRIENDS 100

typedef struct {
    char name[MAX_NAME_LEN];
} Person;

void processFriends(FILE *file, Person *friends, int *numFriends) {
    char line[MAX_LINE_LEN];
    
    // Khởi tạo số lượng bạn bè
    *numFriends = 0;

    while (*numFriends < MAX_FRIENDS && fgets(line, sizeof(line), file) != NULL) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // Xử lý và lưu trữ thông tin bạn bè vào mảng
        char *token = strtok(line, "/");
        while (token != NULL) {
            strcpy(friends[*numFriends].name, token);
            (*numFriends)++;
            token = strtok(NULL, "/");
        }
    }
}

int main() {
    FILE *file = fopen("friend.txt", "r");
    
    if (file != NULL) {
        Person friends[MAX_FRIENDS];
        int numFriends;

        processFriends(file, friends, &numFriends);

        // In ra thông tin bạn bè đã được lưu vào mảng
        for (int i = 0; i < numFriends; i++) {
            printf("Friend %d: %s\n", i + 1, friends[i].name);
        }

        fclose(file);
    } else {
        fprintf(stderr, "Error opening the file.\n");
    }

    return 0;
}
