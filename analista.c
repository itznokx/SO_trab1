#include "analista.h"

sem_t* sem_block;

// 1024 buffer size
#define ALLOC_SIZE 1024

#define nullptr NULL

void start_analist(){
	remove("pidanalista.txt");
	pid_t pid = getpid();
	FILE* pidanalista = fopen("pidanalista.txt","w+");
	if (!pidanalista){
		perror("pidanalista");
	}
	fprintf(pidanalista,"%d",pid);
	fclose(pidanalista);
}
void  rewrite_lng(int buffer_size,int line_counter){
	FILE* lng = fopen("lng.txt", "r");
    if (lng == NULL) {
        perror("Error opening file");
        return;
    }
    FILE *temp_file = fopen("temp.txt", "w");
    if (temp_file == NULL) {
        perror("Error creating temporary file");
        fclose(lng);
        return;
    }
    char buffer[buffer_size];
    int counter = 0;
    while (fgets(buffer,ALLOC_SIZE,lng)!=NULL){
    	if (counter > line_counter){
    		fputs(buffer,temp_file);
    	}
    	counter++;
    }
    fclose(lng);
    fclose(temp_file);
    if (remove("lng.txt") != 0) {
        perror("Erro ao deletar lng.txt");
        return;
    }

    if (rename("temp.txt", "lng.txt") != 0) {
        perror("Erro renomear temp");
        return;
    }
}
void* print_pids(int PID_MAX_PRINT){
	FILE* lng = fopen("lng.txt","r");
	int actualPid;
	int counter = 0;
	char buffer[ALLOC_SIZE];
	while(	(counter < PID_MAX_PRINT)&&
			(fgets(buffer, ALLOC_SIZE, lng) != NULL)
		){
		printf("(Analista) PID: %s\n",buffer);
		counter++;
	}
	fclose(lng);
	return NULL;
}
int main (){
	// Criar o arquivo pidanalista.txt e grava seu pid nele;
	start_analist();
	sem_block = sem_open("/sem_block",O_RDWR);
	if (sem_block == SEM_FAILED) {
        sem_block = sem_open("/sem_block", O_CREAT, 0644, 1);
    }
    raise(SIGSTOP);
    while (1)
    {
    	sem_wait(sem_block);
    	FILE* lng = fopen("lng.txt","r");
    	// Se o arquivo lng não existir;
    	if (!lng){
    		// Libera o semaforo
    		sem_post(sem_block);
    		// Dorme e quando acordado irá "pular" uma iteração (continue)
    		raise(SIGSTOP);
    		continue;
    	}
    	fclose(lng);
    	print_pids(10);
    	rewrite_lng(ALLOC_SIZE,10);
    	sem_post(sem_block);

    	raise(SIGSTOP);
    }
}