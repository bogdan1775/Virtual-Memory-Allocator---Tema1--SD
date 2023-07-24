#include "vma.h"

#include<stdlib.h>
#include<stdio.h>
#include<string.h>


arena_t *alloc_arena(const uint64_t size)
{
    arena_t *arena=malloc(sizeof(arena_t));
    arena->arena_size=size;

    arena->alloc_list=malloc(sizeof(list_t));

    arena->alloc_list->head=NULL;
    arena->alloc_list->data_size=sizeof(block_t);
    arena->alloc_list->size=0;

    return arena;
}


// functiile mele
void my_free_miniblock(dll_node_t* node_miniblock)
{
    miniblock_t* miniblock=node_miniblock->data;
    if(miniblock->rw_buffer!=NULL)
        free(miniblock->rw_buffer);
    
    free(miniblock);
    free(node_miniblock);
}

void my_free_block(dll_node_t* node_block)
{
    block_t* block=node_block->data;
    if(block->miniblock_list)
        free(block->miniblock_list);
    free(block);
    free(node_block);

}



void dealloc_arena(arena_t *arena)
{
    if(arena->alloc_list->head!=NULL){
       // printf("dewfwes");
    dll_node_t* node_block=arena->alloc_list->head;
    dll_node_t* last_block;

    for(size_t i=0;i<arena->alloc_list->size;i++){
        dll_node_t * node_miniblock=((block_t*)node_block->data)->miniblock_list->head;
        dll_node_t* last_miniblock;//last;
        for(size_t j=0;j<((block_t*)node_block->data)->miniblock_list->size;j++){
            last_miniblock=node_miniblock;
            node_miniblock= node_miniblock->next;//??pus minibloch inainte de =
            my_free_miniblock(last_miniblock);
        }

        last_block=node_block;
        node_block=node_block->next;
        my_free_block(last_block);
    
    }
    }
    //if(arena->alloc_list)
    free(arena->alloc_list);
    

    free(arena);

}

void verificare_blockuri_vecine(arena_t *arena )
{
    list_t *list=arena->alloc_list;
    dll_node_t *curr=list->head, *curr2;

    while(curr->next!=NULL){
        curr2=curr->next;

        int address=((block_t*)curr->data)->start_address;
        int address2=((block_t*)curr2->data)->start_address;

        int size=((block_t*)curr->data)->size;
        int size2=((block_t*)curr2->data)->size;
        if(address+size==address2){
            ((block_t*)curr->data)->size=size+size2;
            dll_node_t *node=((block_t*)curr->data)->miniblock_list->head;
            while(node->next!=NULL){
                node=node->next;
            }
            node->next=((block_t*)curr2->data)->miniblock_list->head;
            ((block_t*)curr2->data)->miniblock_list->head->prev=node;

            //scadere nr block-uri
            arena->alloc_list->size--;
            //((block_t*) arena->alloc_list->curr->data)->miniblock_list->s

            //adunare nr miniblokuri
            ((block_t*) curr->data)->miniblock_list->size+=((block_t*) curr2->data)->miniblock_list->size;

            //adunare size
            //((block_t*) curr->data)->size+=((block_t*) curr2->data)->size;

            //eliberare block fara lista de miniblockuri

            if(curr2->next==NULL){
                curr->next=NULL;
            }
            else
                {curr->next=curr2->next;
                curr2->next->prev=curr;}

            free(((block_t*)curr2->data)->miniblock_list);
            free(curr2->data);
            free(curr2);
            if(curr->prev)
                curr=curr->prev;
        }
        else
            curr=curr->next;
    }
}

void add_nth_block(arena_t *arena, size_t n, const uint64_t address, const uint64_t size)
{
    list_t *list=arena->alloc_list;

    //alocare nod de block
    dll_node_t *new_block;
    new_block=malloc(sizeof(dll_node_t));
    new_block->data=malloc(list->data_size);

    ((block_t*)new_block->data)->start_address=address;
    ((block_t*)new_block->data)->size=size;


    ((block_t*)new_block->data)->miniblock_list=malloc(sizeof(list_t));
    ((block_t*)new_block->data)->miniblock_list->data_size=sizeof(miniblock_t);


    //alocare miniblock
    dll_node_t *new_miniblock;
    new_miniblock=malloc(sizeof(dll_node_t));
    new_miniblock->data=malloc(sizeof(miniblock_t));
    ((miniblock_t*)new_miniblock->data)->start_address=address;
    ((miniblock_t*)new_miniblock->data)->size=size;
    ((miniblock_t*)new_miniblock->data)->perm=6;
    ((miniblock_t*)new_miniblock->data)->rw_buffer=NULL;
    new_miniblock->next=NULL;
    new_miniblock->prev=NULL;
    

    ((block_t*)new_block->data)->miniblock_list->head=new_miniblock;
    ((block_t*)new_block->data)->miniblock_list->size=1;
    
    if(n==0){
        new_block->next=list->head;
        new_block->prev=NULL;
        if(list->head)
            list->head->prev=new_block;
        list->head=new_block;
        list->size++;
        return;
    }

    dll_node_t *curr=list->head;
    size_t nr=0;
    while(curr->next!=NULL && nr<n-1){
        curr=curr->next;
         nr++;
        }

    new_block->next=curr->next;
    new_block->prev=curr;

    if(curr->next)
        curr->next->prev=new_block;
    curr->next=new_block;
    
    list->size++;

    

}

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
    //list_t *list=arena->alloc_list;
    dll_node_t *curr=arena->alloc_list->head;
    block_t *block;

    //verificare daca poate fi adaugat
    if(address>=arena->arena_size){
        printf("The allocated address is outside the size of arena\n");
        return;
    }

    if(address+size>=arena->arena_size){
        printf("The end address is past the size of the arena\n");
        return;
    }

    while(curr!=NULL){
        block=curr->data;
        if(block->start_address+block->size >address && block->start_address<address+size){
            printf("This zone has already allocated\n");
            return;
        }
        curr=curr->next;
    }

    //incepere adaugare

    curr=arena->alloc_list->head;
    size_t n=0;

    while(curr!=NULL && ((block_t*)curr->data)->start_address<address){
        n++;
        curr=curr->next;
    }
    //adaugare block
    add_nth_block(arena,n,address,size);

    //unire blockuri daca sunt vecine
    verificare_blockuri_vecine(arena);
    
} 

dll_node_t* node_create(const void* new_data, unsigned int data_size)
{
    dll_node_t *new;
    new=malloc(sizeof(dll_node_t));
    //DIE(new==NULL,"Nu s-a putut creea noul nod");

    new->data=malloc(data_size);
    //DIE(new->data==NULL,"Nu s-a putut aloca memorie pentru data");

    memcpy(new->data,new_data,data_size);

    return new;
}

dll_node_t*
dll_get_nth_node(list_t* list, unsigned int n)
{
	/* TODO */
    if(n>=list->size)
        n=n-1;
    dll_node_t *curr=list->head;
    int nr=0;

    while(nr<n){
        curr=curr->next;
        nr++;
    }

    return curr;
}

dll_node_t*
dll_remove_nth_node(list_t* list, unsigned int n)
{
	/* TODO */
    if(list==NULL || list->size==0)
        return NULL;

    if(n>=list->size)
        n=list->size-1;

    if(n==0){
        dll_node_t* curr;
        curr=list->head;

        list->head=list->head->next;
        if(list->head)
            list->head->prev=NULL;

        list->size--;
        return curr;
    }

    dll_node_t* curr,*last;
    int nr=0;

    curr=list->head;
    last=list->head;

    while(curr->next !=NULL && nr<n-1){
        curr=curr->next;
        nr++;
    }

    last=curr->next;//nodul care trebuie eliminat

    curr->next=last->next;
    if(last->next!=NULL)
        last->next->prev=curr;

    list->size--;
    return last;


}

void
dll_add_nth_node(list_t* list, unsigned int n, const void* new_data)
{
	/* TODO */
    if(list==NULL)
        return;

    dll_node_t *new;
    new=node_create(new_data,list->data_size);

    if(n>list->size)
        n=list->size;

    if(list->size==0 || n==0){
        new->next=list->head;
        new->prev=NULL;
        if(list->head)
            list->head->prev=new;
        
        list->head=new;

        list->size++;
    }

    else{
        dll_node_t *curr=list->head;
        int nr=0;
        while(curr->next!=NULL && nr<n-1){
            curr=curr->next;
            nr++;
        }

        new->next=curr->next;
        new->prev=curr;

        if(curr->next)
            curr->next->prev=new;
        curr->next=new;

        list->size++;
    }
}


int cautare_poz(list_t* list_mb,int n,const uint64_t address)
{
    dll_node_t* node=list_mb->head;

    for(int i=0;i<n;i++){
        if(((miniblock_t*) node->data)->start_address==address)
            return i;
        node=node->next;
    }
    return -1;
}

/*void my_free_miniblock(dll_node_t* node_miniblock)
{
    miniblock_t* miniblock=node_miniblock->data;
    if(miniblock->rw_buffer)
        free(miniblock->rw_buffer);
    
    free(miniblock);
    free(node_miniblock);
}

void my_free_block(dll_node_t* node_block)
{
    block_t* block=node_block->data;
    if(block->miniblock_list)
        free(block->miniblock_list);
    free(block);
    free(node_block);

}*/

int calculate_size(list_t* list_mb,int poz)
{
    dll_node_t* node=list_mb->head;
    int nr=0;
    int size=0;
    while(nr<poz){
        nr++;
        size+=((miniblock_t*) node->data)->size;
        node=node->next;
    }
    return size;
}

void free_block(arena_t *arena, const uint64_t address)
{
    //verificare adresa

    dll_node_t *node=arena->alloc_list->head;
    size_t ok=0;
    while(node){
        block_t *block=node->data;
        if(address>=block->start_address && address<block->start_address+block->size)
            ok=1;
        node=node->next;
    }

    if(ok==0){
        printf("Invalid address for free.\n");
        return;
    }

    node=arena->alloc_list->head;
    int poz_block=0;

    while(address<((block_t*)node->data)->start_address){
        node=node->next;
        poz_block++;
    }
    
    //incepere eliminare

    list_t *list_mb=((block_t*)node->data)->miniblock_list;
    block_t *block=node->data;

    int poz_minib=cautare_poz(list_mb,block->miniblock_list->size ,address);
    if(poz_minib==-1){
        printf("Invalid address for free.\n");
        return;
    }

    //eliminare un miniblock =>eliminare block
    if(block->miniblock_list->size==1){

        block->miniblock_list->size=0;
        my_free_miniblock(block->miniblock_list->head);
        dll_node_t* removed_block=dll_remove_nth_node(arena->alloc_list,poz_block);

        my_free_block(removed_block);
        return;
    }

    //dll_node_t* removed_minib=dll_get_nth_node(list_mb,poz_minib);
    dll_node_t* removed_minib=dll_remove_nth_node(list_mb,poz_minib);

    //eliminare primul miniblock dintr-un block
    if(poz_minib==0){
        //block->miniblock_list->size--;

        block->size-=((miniblock_t*) removed_minib->data)->size;
        block->start_address=((miniblock_t*)list_mb->head->data)->start_address;

        //list_mb->head=list_mb->head->next;
        
        my_free_miniblock( removed_minib);
        //free(removed_minib);
        return;
    }

    //eliminate miniblock de la finalul unui block
    if(poz_minib==list_mb->size){
        //block->miniblock_list->size--;

        block->size-=((miniblock_t*) removed_minib->data)->size;
        //my_free_miniblock((removed_minib->data));
        //free(removed_minib);
        my_free_miniblock(removed_minib);
        return;
    }
    
    // elimininare miniblock mijloc

    int size_before_poz=calculate_size(list_mb,poz_minib);


    //dll_node_t* node_mb_head=dll_get_nth_node(list_mb,poz_minib);
    //alocare nod de block
    dll_node_t *new_block;
    new_block=malloc(sizeof(dll_node_t));
    new_block->data=malloc(arena->alloc_list->data_size);

    ((block_t*)new_block->data)->start_address=((miniblock_t*)removed_minib->next->data)->start_address;
    ((block_t*)new_block->data)->size=block->size-size_before_poz-((miniblock_t*)removed_minib->data)->size;

    block->size=size_before_poz;
    

    //node_mb_head->prev->next=NULL;
    //node_mb_head->prev=NULL;
    //removed_minib->prev->next=NULL;

    ((block_t*)new_block->data)->miniblock_list=malloc(sizeof(list_t));
    ((block_t*)new_block->data)->miniblock_list->data_size=sizeof(miniblock_t);
    ((block_t*)new_block->data)->miniblock_list->head=removed_minib->next;
    removed_minib->next->prev=NULL;

    ((block_t*)new_block->data)->miniblock_list->size=block->miniblock_list->size-poz_minib;
    block->miniblock_list->size=poz_minib;

    //free_miniblock(((miniblock_t*) removed_minib->data));
    //free(removed_minib);

    my_free_miniblock(removed_minib);
    //add_nth_block(arena->alloc_list,poz_block,const (new_block->data));

    new_block->next=node->next;
    node->next->prev=new_block;
    node->next=new_block;
    new_block->prev=node;
    arena->alloc_list->size++;


}

void read(arena_t *arena, uint64_t address, uint64_t size)
{
    dll_node_t *curr=arena->alloc_list->head;
    block_t *block;
    int ok=0;
    // verificare daca adresa este aloca
    while(curr!=NULL){
        block=curr->data;
        if(address>=block->start_address && address<block->start_address+block->size){
            ok=1;
            break;
        }
        curr=curr->next;
    }
    if(ok==0){
        printf("Invalid address for read.\n");
        return;
    }

    curr=arena->alloc_list->head;
    while(address>((block_t*)curr->data)->start_address+((block_t*)curr->data)->size)
        curr=curr->next;
    
    block=curr->data;

    //verificare daca depaseste zona alocata
    uint64_t size2=size;
    if(address+size>block->start_address+block->size){
        size2=block->start_address+block->size-address;
        printf("Warning: size was bigger than the block size. Reading %lu characters.\n",size2);
    }

   
    //incepere citire;  
    dll_node_t*node_minib=block->miniblock_list->head;
    miniblock_t*miniblock=node_minib->data;
    int read_size=0,ok2=1;
    int decalaj=0,decalaj2=0;
    while(read_size<size2){
        miniblock=node_minib->data;
        if(address<miniblock->start_address+miniblock->size){
            decalaj=(int)address-miniblock->start_address;
            if(decalaj<0)
                decalaj=0;
            decalaj2=miniblock->size;
            //if(address>miniblock->start_address)
                //decalaj2=address-miniblock->start_address;
            if(size2-read_size<decalaj2)
                decalaj2=size2-read_size;
            char* sir=calloc(((int)miniblock->size)+1,sizeof(char));
            strncpy(sir,((char*) miniblock->rw_buffer)+decalaj*sizeof(char),decalaj2);
            read_size+=decalaj2;
            printf("%s",sir);
            free(sir);
        }
        node_minib=node_minib->next;
    }
    printf("\n");
}

void write(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data)
{
    dll_node_t *curr=arena->alloc_list->head;
    block_t *block;
    int ok=0;
    // verificare daca adresa este aloca
    while(curr!=NULL){
        block=curr->data;
        if(address>=block->start_address && address<block->start_address+block->size){
            ok=1;
            break;
        }
        curr=curr->next;
    }
    if(ok==0){
        printf("Invalid address for write.\n");
        return;
    }

    curr=arena->alloc_list->head;
    while(address>((block_t*)curr->data)->start_address+((block_t*)curr->data)->size)
        curr=curr->next;
    
    block=curr->data;

    //verificare daca depaseste zona modificata
    uint64_t size2=size;
    if(address+size>block->start_address+block->size){
        size2=block->start_address+block->size-address;
        printf("Warning: size was bigger than the block size. Writing %lu characters.\n",size2);
    }

    //incepere scriere;  
    dll_node_t*node_minib=block->miniblock_list->head;
    miniblock_t*miniblock=node_minib->data;
    int write_size=0,ok2=1;
    int decalaj=0,decalaj2=0;
    while(write_size<size2){
        miniblock=node_minib->data;
        if(address<miniblock->start_address+miniblock->size){
            decalaj=(int)address-miniblock->start_address;
            if(decalaj<0)
                decalaj=0;
            decalaj2=miniblock->size;
            if(decalaj2>size2-write_size)
                decalaj2=size2-write_size;
            //if(address>miniblock->start_address)
                //decalaj2=address-miniblock->start_address;
            //if(size-write_size<decalaj2)
                //decalaj2=size-write_size;
            miniblock->rw_buffer=malloc(miniblock->size);
            strncpy(((char*) miniblock->rw_buffer)+decalaj,((char*)data)+write_size,
                (int)decalaj2);

            write_size+=decalaj2;
        }
        node_minib=node_minib->next;
    }

}

int det_nr_miniblocks(const arena_t *arena)
{
    dll_node_t*node=arena->alloc_list->head;
    int count=0;
    while(node){
        //count+=((miniblock_t*)node->data)->size;
        count+=((block_t*)node->data)->miniblock_list->size;
        node=node->next;
    }
    return count;
}

uint64_t memory_use(const arena_t* arena)
{
    uint64_t memory=0;
    dll_node_t* node=arena->alloc_list->head;
    for(int i=0;i<arena->alloc_list->size;i++)
    {
        memory=memory+((block_t*)node->data)->size;
        node=node->next;
    }
    return memory;
}

void pmap(const arena_t *arena)
{
    list_t *list_b=arena->alloc_list;
    dll_node_t*node=arena->alloc_list->head;
    printf("Total memory: 0x%lX bytes\n",arena->arena_size);
    uint64_t free_mem=arena->arena_size-memory_use(arena);
    printf("Free memory: 0x%lX bytes \n",free_mem);
    printf("Number of allocated blocks: %ld\n",arena->alloc_list->size);
    int nr_minib=det_nr_miniblocks(arena);
    printf("Number of allocated miniblocks: %d\n",nr_minib);

    for(int i=0;i<arena->alloc_list->size;i++){
        block_t *block=node->data;
        printf("\n\nBlock %d begin\n",i+1);
        printf("Zone: 0x%lX - 0x%lX\n",block->start_address,block->start_address+block->size);

        dll_node_t* node_mb=block->miniblock_list->head;
        for(int j=0;j<block->miniblock_list->size;j++){
            miniblock_t* minib=node_mb->data;
            printf("Miniblock %d:",j+1);
            printf("    0x%lX    -   0x%lX    ",minib->start_address,minib->start_address+minib->size);
            printf(" | RW-\n");
            node_mb=node_mb->next;
        
        }
        printf("Block %d end\n",i+1);
        node=node->next;
    }
}


void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{

}
