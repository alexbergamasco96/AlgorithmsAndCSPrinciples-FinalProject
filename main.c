#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXLENNAME 255
#define MAXHEIGHT 255
#define MAXSONS 1024
#define DIR 0
#define FILE 1
#define DIM_HASH 79


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////			STRUTTURE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef struct node_s {
	char *name;
	struct node_s *padre;
	int type;
	int figli;                          //only DIR
	int altezza;
	char * addr;
	char * contenuto;                   //only FILE
	struct node **hashtab;
	struct node *next;                  //lista concatenata per la tabella hash
} node_t;


/////////////////////////////////////
/////           PROTOTIPI
/////////////////////////////////////

int hash_function(char *);
node_t * retrieve_node(node_t *, char *);
node_t * figlio_in_dir(node_t *, char *);
char * the_new_name(char* );
char * sistema_contenuto(char * );
void create(node_t *, char * );
void create_dir(node_t *, char * );
void write(node_t * , char* , char *);
void read(node_t * , char* );
void delete_(node_t *, char *);
void delete_r(node_t *, char *);
void find(node_t *, char *);
void find_r(node_t *, char *);
int strcmp_for_quicksort(const void *name1, const void *name2);


/////////////////////////////////////
/////           MAIN
/////////////////////////////////////

int main() {
    char *addr;
	char *istr;
	int flag;

	//creo/alloco root
    node_t *root;
    root = (node_t *)malloc(sizeof(node_t));
    root->name = (char *)calloc(1, sizeof(char));
    root->padre = NULL;
    root->type = DIR;
    root->altezza = 0;
    root->addr = NULL;
    root->figli = 0;
    root->contenuto = NULL;
    root->hashtab = calloc(DIM_HASH, sizeof(node_t *));


    //acquisizione dati

    flag = 0;                    //uscire dal ciclo in presenza di exit

	istr = (char *)calloc(1, sizeof(char)*100);
	addr = (char *)calloc(1, sizeof(char)*4000);
	while(!flag){
        scanf("%s", istr);
        if(istr != NULL){
            if (strcmp(istr, "create") == 0) {
                scanf("%s", addr);
                create(root, addr);
            }
            else if (strcmp(istr, "create_dir") == 0) {
                scanf("%s", addr);
                create_dir(root, addr);
            }
            else if (strcmp(istr, "read") == 0) {
                scanf("%s", addr);
                read(root, addr);
            }
            else if (strcmp(istr, "write") == 0) {
                scanf("%s", addr);
                char *contenuto = (char *)calloc(1,sizeof(char)*10000);
                gets(contenuto);                    //ricorda di sistemare contenuto
                write(root, addr, contenuto);
            }
            else if (strcmp(istr, "delete") == 0) {
                scanf("%s", addr);
                delete_(root, addr);
            }
            else if (strcmp(istr, "delete_r") == 0) {
                scanf("%s", addr);
                delete_r(root, addr);
            }
            else if (strcmp(istr, "find") == 0) {
                scanf("%s", addr);
                find(root, addr);
            }
            else if (strcmp(istr, "exit") == 0) {
                flag=1;
            }
        }
	}
	//fine ciclo operazioni
    free(istr);
    free(addr);
    free(root->name);
    free(root);


	return 0;
}

/////////////////////////////////////
/////       FUNZIONI HASH
/////////////////////////////////////


/*  Find this hash function here: http://www.cse.yorku.ca/~oz/hash.html
    Dan Bernstein Function
*/
int hash_function(char *str){
	unsigned long hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    size_t idx = hash % DIM_HASH;
	return idx;		 //The func return a long int that isn't good for our hash_table. Use operator % to index-link in the table (hash%capacity)
}

///////////////////////////////////////////
/////           FUNZIONI GENERALI
///////////////////////////////////////////

node_t *retrieve_node(node_t *node, char *addr) {
    char *path = strdup(addr);
    char *curr_token = strtok(path, "/");         //path del primo figlio
    node_t *padre = node;                         //inizialmente padre punta alla radice
    char *next_token =  strtok(NULL, "/");
    node_t *ctrl;
    while (next_token){
        ctrl = figlio_in_dir(padre, curr_token);
        if(!ctrl){
            return NULL;
        }else{
            padre = ctrl;
            curr_token =  next_token;
            next_token = strtok(NULL, "/");
        }
    }
    return padre;
}

node_t *figlio_in_dir(node_t* padre, char* name){
    node_t* figlio = NULL;
    if (!padre->figli){
        return NULL;
    }
    int idx;
    idx = hash_function(name);
    figlio = padre->hashtab[idx];

    while(figlio != NULL && strcmp(figlio->name, name) != 0){       //scorri lista concatenata
        if(figlio->next == NULL){
            return NULL;
        }
        figlio = figlio->next;
    }

    return figlio;
}

char * the_new_name(char * path_r) {
    char *path = strdup(path_r);
	char *token;
	char *next_token;
	token = strtok(path, "/");
	next_token = strtok(NULL, "/");
	while (next_token != NULL) {
		token = next_token;
		next_token = strtok(NULL, "/");
	}
	return token;
}



char * sistema_contenuto(char * contenuto){         //scorrere spazione rimuovere ""
    char * contenuto_okkey;
    contenuto_okkey = (char *)malloc(sizeof(char)*2500);
    int l=strlen(contenuto);
    int i=2;
    int a=0;
    while(i < (l-1)){
        contenuto_okkey[a]=contenuto[i];
        a++;
        i++;
    }
    contenuto_okkey[a] = '\0';
    return contenuto_okkey;
}

void create(node_t *root, char * addr_r){
	node_t *last_dir;
	char *addr = strdup(addr_r);
	char *filename = NULL;
	int file_len;

	last_dir = retrieve_node(root, addr);
	filename = the_new_name(addr);
	if(!last_dir || last_dir->type != DIR || !filename){        //controllo struttura
        printf("no\n");
        //free(addr);
        return ;
	}
    file_len = strlen(filename);
    if(file_len > MAXLENNAME || last_dir->figli >= MAXSONS || last_dir->altezza >= MAXHEIGHT){        //controllo limiti FS
        printf("no\n");
        //free(addr);
        return;
    }
    node_t * new_ = calloc(1, sizeof(node_t));
    new_->name = strdup(filename);
    int idx = hash_function(filename);              //idx della key
    if(last_dir->hashtab[idx] == NULL){
        new_->addr = strdup(addr_r);
        new_->altezza = last_dir->altezza + 1;
        new_->contenuto = NULL;
        new_->padre = last_dir;
        new_->type = FILE;
        new_->figli = 0;
        new_->next = NULL;
        last_dir->hashtab[idx] = new_;
        last_dir->figli = last_dir->figli + 1;
        printf("ok\n");
        return;
    }else{
        node_t *ctrl = figlio_in_dir(last_dir, filename);
        if(!ctrl){                                                   //figlio non esiste, procedo all'inserimento
            ctrl = last_dir->hashtab[idx];
            while(ctrl->next != NULL){
                ctrl = ctrl->next;
            }
            ctrl->next = new_;
            new_->addr = strdup(addr_r);
            new_->altezza = last_dir->altezza + 1;
            new_->contenuto = NULL;
            new_->padre = last_dir;
            new_->type = FILE;
            new_->figli = 0;
            new_->next = NULL;
            last_dir->figli = last_dir->figli + 1;
            printf("ok\n");
            return;
        }
        printf("no\n");
        return;
    }
    free(new_->name);
    free(new_);
}


void create_dir(node_t *root, char * addr_r){
	node_t *last_dir = root;
	char *addr = strdup(addr_r);
	char *dirname = NULL;
	int dir_len;

	last_dir = retrieve_node(root, addr);
	dirname = the_new_name(addr);
	if(!last_dir || last_dir->type != DIR || !dirname){        //controllo struttura
        printf("no\n");
        //free(addr);
        return ;
	}
    dir_len = strlen(dirname);
    if(dir_len > MAXLENNAME || last_dir->figli >= MAXSONS || last_dir->altezza >= MAXHEIGHT){        //controllo limiti FS
        printf("no\n");
        //free(addr);
        return;
    }
    node_t * new_ = calloc(1, sizeof(node_t));
    new_->name = strdup(dirname);
    int idx = hash_function(dirname);              //idx della key
    if(last_dir->hashtab[idx] == NULL){
        new_->addr = strdup(addr_r);
        new_->altezza = last_dir->altezza + 1;
        new_->hashtab = calloc(DIM_HASH, sizeof(node_t *));
        new_->padre = last_dir;
        new_->type = DIR;
        new_->figli = 0;
        new_->next = NULL;
        last_dir->hashtab[idx] = new_;
        last_dir->figli = last_dir->figli + 1;
        printf("ok\n");
        return;
    }else{
        node_t *ctrl = figlio_in_dir(last_dir, dirname);
        if(!ctrl){                                                   //figlio non esiste, procedo all'inserimento
            ctrl = last_dir->hashtab[idx];
            while(ctrl->next != NULL){
                ctrl = ctrl->next;
            }
            ctrl->next = new_;
            new_->addr = strdup(addr_r);
            new_->altezza = last_dir->altezza + 1;
            new_->hashtab = calloc(DIM_HASH, sizeof(node_t *));
            new_->padre = last_dir;
            new_->type = DIR;
            new_->figli = 0;
            new_->next = NULL;
            last_dir->figli = last_dir->figli + 1;
            printf("ok\n");
            return;
        }
        printf("no\n");
        return;
    }
    free(new_->name);
    free(new_);
}

void write(node_t * padre, char* path_r, char *contenuto){
    node_t *file = NULL;
    node_t *last_dir = NULL;
    char *path = strdup(path_r);
    char *filename = the_new_name(path);
    last_dir = retrieve_node(padre, path_r);
    if(!last_dir){
        printf("no\n");
        return;
    }
    file = figlio_in_dir(last_dir, filename);
    if(!file || file->type != FILE){
        printf("no\n");
        return;
    }
    char * contenuto_ok = sistema_contenuto(contenuto);
    if(file->contenuto == NULL){
        file->contenuto = strdup(contenuto_ok);
        int contlen = strlen(contenuto_ok);
        printf("ok %d\n", contlen);
    }
    else{
        free(file->contenuto);
        file->contenuto = strdup(contenuto_ok);
        int contlen = strlen(contenuto_ok);
        printf("ok %d\n", contlen);
    }
    return;
}

void read(node_t * padre, char* path){
    node_t *file = NULL;
    node_t *last_dir = NULL;
    last_dir = retrieve_node(padre, path);
    if(!last_dir){
        printf("no\n");
        return;
    }
    char * filename = the_new_name(path);
    file = figlio_in_dir(last_dir, filename);
    if(!file || file->type != FILE){
        printf("no\n");
        return;
    }
    if(file->contenuto){
        printf("contenuto %s\n", file->contenuto);
        return;
    }else{
        printf("contenuto \n");
        return;
    }
}


void delete_(node_t *padre, char *path){
    //char *path_ = strdup(path_r);
    //char *path = strdup(path_r);
    node_t * last_dir = retrieve_node(padre, path);
    char *del_name = the_new_name(path);
    if(!last_dir){
        printf("no\n");
        return;
    }
    //accedi all'elemento tenendo conto dei possibili concatenamenti per collisione
    //figlio_in_dir tenendo conto del precedente
    int idx = hash_function(del_name);
    node_t* curr = last_dir->hashtab[idx];
    node_t* prec = NULL;
    if(!del_name){
        printf("no\n");
        return;
    }
    while(curr != NULL && strcmp(curr->name, del_name) != 0){
        prec = curr;
        curr = curr->next;
    }
    if(curr){
        if(curr->figli == 0){
            if(!prec){ //curr è il primo in quell'indice
                last_dir->hashtab[idx] = curr->next;
            }
            else{
                prec->next = curr->next;
            }
            if(curr->type == DIR){
                free(curr->hashtab);
            }
            if(curr->type == FILE && curr->contenuto != NULL){
                free(curr->contenuto);
            }
            last_dir->figli --;
            free(curr->name);
            free(curr->addr);
            free(curr);
            printf("ok\n");
            return;
        }
        printf("no\n");
        return;
    }
    printf("no\n");
    return;
}

void delete_r(node_t *padre, char *path){
    node_t * last_dir = retrieve_node(padre, path);
    char *del_name = the_new_name(path);
    if(!last_dir){
        printf("no\n");
        return;
    }
    //accedi all'elemento tenendo conto dei possibili concatenamenti per collisione
    //figlio_in_dir tenendo conto del precedente

    int idx = hash_function(del_name);
    node_t* curr = last_dir->hashtab[idx];
    node_t* prec = NULL;
    while(curr != NULL && strcmp(curr->name, del_name) != 0){
        prec = curr;
        curr = curr->next;
    }
    if(curr){
        if(!prec){ //curr è il primo in quell'indice
            last_dir->hashtab[idx] = curr->next;
        }
        if(prec){
            prec->next = curr->next;
        }
        //ricorsivamente vado sui figli
        if(!delete_loop(curr)){
                printf("no\n");
                return;
        }
        last_dir->figli --;
        printf("ok\n");
        return;
    }
    printf("no\n");
    return;
}


// visita post order
// elimino prima figli poi next
int delete_loop(node_t *node){
    if(!node){
        return 0;
    }
    int i;
    //Post-Order : prima figli
    if(node->figli){
        for(i = 0 ; i < DIM_HASH ; i++){
            if(node->hashtab[i]){
                delete_loop(node->hashtab[i]);
            }
        }

    }
    //Poi Next
    node_t * succ;
    succ = node->next;
    if(succ){
        delete_loop(succ);
    }
    //dealloco nodo
    if(node->type == FILE && node->contenuto){
        free(node->contenuto);
        free(node->name);
    }
    if(node->type == DIR){
        free(node->hashtab);
        free(node->name);
    }
    free(node->addr);
    free(node);
    return 1;
}

//Variabili Globali per gestire il numero di risorse trovate con quel nome e, inserendole nell'array, riuscire ad ordinarle lessicograficamente

int res_find;
char **find_array;

void find(node_t *root, char *name){
    res_find = 0;
    find_array = calloc(500, sizeof(char *));
    find_r(root, name);
    qsort(find_array, res_find, sizeof(char *), strcmp_for_quicksort);          //quicksort in stdlib
    if(res_find > 0){
        int i;
        for(i = 0 ; i < res_find ; i++){
            printf("ok %s\n", find_array[i]);
        }
    }
    else{
        printf("no\n");
    }
    res_find = 0;
    free(find_array);
    return;
}

void find_r(node_t * node, char *name){
    if(!node){
        return;
    }
    if(!strcmp(node->name, name)){
        find_array[res_find] = strdup(node->addr);
        res_find ++;
    }
    if(node->figli){                            //prima figli
        int i;
        for(i=0; i < DIM_HASH; i++){
            find_r(node->hashtab[i], name);
        }
    }
    find_r(node->next, name);                   //poi next
    return;
}


int strcmp_for_quicksort(const void *name1, const void *name2){         //funzione comparazione per quicksort
    int ris;
    const char *name1_ = *(const char **)name1;
    const char *name2_ = *(const char **)name2;
    ris = strcmp(name1_, name2_);
    return ris;
}







