#include "clist.h"

typedef struct List {
	char *name;
	char *ip;
	struct List* urm;
}List;

List *addUser(List *list, char *name, char *ip) {
	List *item=(List*)malloc(sizeof(List));
    if(!item){
        printf("memorie insuficienta");
        exit(EXIT_FAILURE);
        }
    item->ip=ip;
	item->name=name;
    item->urm=list;

	return item;
}

void deleteUser(List *list, char *name) {
	List *pred; //predecessor
	List *crt; //current user

	for(pred=NULL,crt=list;crt;pred=crt,crt=crt->urm){
        if(strcmp(crt->name,name) == 0){
            if(pred==NULL){     
                list=list->urm;
                }else{      
                pred->urm=crt->urm;
                }
            free(crt);
        }
    }
}

void getUser(List *list, char *name, char *buff){
	if(list == NULL){
		return ;
	}

	if(strcmp(list->name, name) == 0){
		strcpy(buff, list->ip);
		return ;
	}

	 getUser(list->urm, name, buff);
}

int existUser(List *list, char *name) {
	if(list == NULL) {
		return 0;
	} else if(strcmp(list->name,name) == 0){
		return 1;
	} else {
		return existUser(list->urm, name);
	}
}

void listUsers(List *list, char *buf){
    for(;list;list=list->urm){
       if(list->ip != NULL){
		strcat(buf,list->name);
		strcat(buf,", ");
        }
	}
}

void freeList(List *list){
    List *p;
    while(list){
        p=list->urm;
        free(list);
        list=p;
    }
}
