#include <stdlib.h>
#include <string.h>

typedef struct List List;

List *addUser(List *urm, char *name, char *ip);
void deleteUser(List *list, char *name);
void getUser(List *list, char *name, char *buff);
int existUser(List *list, char *name);
void listUsers(List *list, char *buf);
void freeList(List *list);