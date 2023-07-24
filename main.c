#include "vma.h"
#include<stdio.h>
#include<string.h>
#define NMAX 100

/*void verificare_com_alloc_arena(char* comanda,arena_t** arena){
    char *p;
    char parametru1[NMAX];
    p=strtok(comanda," \n");
    if(!p){
        printf("Invalid command. Please try again.\n");
        return;
    }

    strcpy(parametru1,p);
    p=strtok(NULL,"\n ");

    if(p!=NULL){
        printf("Invalid command. Please try again.\n");
        return;
    }

    uint64_t size=atoi(parametru1);
    //printf("%ld",size);
    *arena=alloc_arena(size);

}

void verificare_com_dealloc_block(char* comanda,arena_t* arena){
    char *p;
    char parametru1[NMAX];
    p=strtok(comanda," \n");
    if(!p){
        printf("Invalid command. Please try again.\n");
        return;
    }

    strcpy(parametru1,p);
    p=strtok(NULL,"\n ");

    if(p!=NULL){
        printf("Invalid command. Please try again.\n");
        return;
    }

    uint64_t size=atoi(parametru1);
    //printf("%ld",size);
    free_block(arena,size);

}

void verificare_com_alloc_block(char *comanda, arena_t* arena)
{
    char *p;
    char param1[NMAX],param2[NMAX];
    
    p=strtok(comanda,"\n ");
    if(p==NULL){
        printf("Invalid command. Please try again.\n");
        return;
    }

    strcpy(param1,p);
    p=strtok(NULL,"\n ");
    if(p==NULL){
        printf("Invalid command. Please try again.\n");
        return;
    }

    strcpy(param2,p);
    p=strtok(NULL,"\n ");
    if(p!=NULL){
        printf("Invalid command. Please try again.\n");
        return;
    }

    uint64_t address=atoi(param1);
    uint64_t size=atoi(param2);
    alloc_block(arena,address,size);

}

void verificare_com_write(char *comanda, arena_t* arena)
{
    char *p;
    char param1[NMAX],param2[NMAX],param3[NMAX];
    
    p=strtok(comanda,"\n ");
    if(p==NULL){
        printf("Invalid command. Please try again.\n");
        return;
    }

    strcpy(param1,p);
    p=strtok(NULL,"\n ");
    if(p==NULL){
        printf("Invalid command. Please try again.\n");
        return;
    }

    strcpy(param2,p);
    p=strtok(NULL,"\n");
    if(p==NULL){
        printf("Invalid command. Please try again.\n");
        return;
    }

    strcpy(param3,p);
    //p=strtok(NULL,"\n ");
    //if(p!=NULL){
       // printf("Invalid command. Please try again.\n");
        //return;
    //}

    uint64_t address=atoi(param1);
    uint64_t size=atoi(param2);
    write(arena,address,size,param3);

}


void verificare_com_read(char *comanda, arena_t* arena)
{
    char *p;
    char param1[NMAX],param2[NMAX];
    
    p=strtok(comanda,"\n ");
    if(p==NULL){
        printf("Invalid command. Please try again.\n");
        return;
    }

    strcpy(param1,p);
    p=strtok(NULL,"\n ");
    if(p==NULL){
        printf("Invalid command. Please try again.\n");
        return;
    }

    strcpy(param2,p);
    p=strtok(NULL,"\n ");
    if(p!=NULL){
        printf("Invalid command. Please try again.\n");
        return;
    }

    uint64_t address=atoi(param1);
    uint64_t size=atoi(param2);
    read(arena,address,size);

}
*/
int main(void)
{
    char comanda[NMAX],comanda1[NMAX],*p;
    //char s[NMAX];
    //fgets(comanda,100,stdin);
   // p=strtok(comanda," \n");
   // strcpy(comanda1,p);
   // p=strtok(NULL," \n");

    arena_t *arena;

    int ok=1;
    while(ok){

        //fgets(comanda,100,stdin);
        //p=strtok(comanda," \n");
       // strcpy(comanda1,p);
       // p=strtok(NULL,"\n");
        //printf("%s\n",p);
        scanf("%s",comanda1);

        if(strcmp(comanda1,"ALLOC_ARENA")==0){
            uint64_t size;
            scanf("%ld",&size);
            //verificare_com_alloc_arena(p,&arena);
           arena=alloc_arena(size);
        }

        else if(strcmp(comanda1,"DEALLOC_ARENA")==0){
           // if(p){
             //    printf("Invalid command. Please try again.\n");
            //}
            //else{
            dealloc_arena(arena);
            ok=0;
            //}
        }

        else if(strcmp(comanda1,"ALLOC_BLOCK")==0){
            uint64_t address;
            uint64_t size;
            //verificare_com_alloc_block(p,arena);
            scanf("%ld",&address);
            scanf("%ld",&size);
            alloc_block(arena,address,size);
        }

        else if(strcmp(comanda1,"FREE_BLOCK")==0){
            uint64_t size;
            scanf("%ld",&size);

            //verificare_com_dealloc_block(p,arena);
            free_block(arena, size);
        }

        else if(strcmp(comanda1,"READ")==0){
            uint64_t address=0;
            uint64_t size=0;
            scanf("%ld",&address);
            scanf("%ld",&size);
            //verificare_com_read(p,arena);
            read(arena,address,size);
        }

        else if(strcmp(comanda1,"WRITE")==0){
            uint64_t address=0;
            uint64_t size=0;
            void*data;
            scanf("%ld",&address);
            scanf("%ld",&size);
            data=malloc(size);
            scanf("%s",data);
            write(arena,address,size,data);
            free(data);
           //verificare_com_write(p,arena);
        }

        else if(strcmp(comanda1,"PMAP")==0){
            //if(p)
                // printf("Invalid command. Please try again.\n");
        
            //else
            pmap(arena);
        }

        else if(strcmp(comanda1,"MPROTECT")==0){
            uint64_t addresss=0;
            int8_t *permission=NULL;
            //mprotect(arena,addresss,permission);
        }

        else
            printf("Invalid command. Please try again.\n");

        
        
    }

    return 0;
}
