#include "analista.h"

sem_t* sem_block;

// 1024 buffer size
#define ALLOC_SIZE 1024

#define nullptr NULL

void* allocmem (size_t size){
	//TODO
}
void fillmem (void* ptr,size_t size){
	memset(ptr,0,size);
}
void start_analist(){
	remove("pidanalista.txt");
	pid_t pid = getpid();
	FILE* pidanalista = fopen("pidanalista.txt","w+");
	fprintf(pidanalista,"%d",pid);
	fclose(pidanalista);
}
void  rewrite_lng(int buffer[],int c){
	remove("lng.txt");
	FILE* lng = fopen("lng.txt","w+");
	int counter = 0;
	while (counter < c) {
		fscanf(lng,"%d\n",&buffer[counter]);
		counter++;
	}
	fclose(lng);
}
void* print_pids(int PID_MAX_PRINT){
	FILE* lng = fopen("lng.txt","w+")
	int actualPid;
	int counter = 0;

	while (counter < PID_MAX_PRINT && fscanf(lng,"%d",&actualPid)!=EOF){
		printf("(Analista) PID: %d\n",actualPid);
		counter++;
	}
	if (PID_MAX_PRINT==0||counter==0){
		fclose(lng);
		return nullptr;
	}
	int rCounter = 0;
	int linesBuffer[ALLOC_SIZE];
	while (fscanf(lng, "%d\n",&linesBuffer[rCounter])==1){
		rCounter++;
	}
	fclose(lng);
	rewrite_lng(linesBuffer,rCounter);
	return nullptr;
}
int main (){
	// Criar o arquivo pidanalista.txt e grava seu pid nele;
	start_analist();
	sem_block = sem_open("/sem_block",O_RDWR);
	if (sem_block == SEM_FAILED) {
        perror("sem_block");
        exit(1);
    }
    raise(SIGSTOP);
    while (1)
    {
    	sem_wait(sem_block);
    	FILE* lng = fopen("lng.txt","w+");
    	// Se o arquivo lng não existir;
    	if (!lng){
    		// Libera o semaforo
    		sem_post(sem_block);
    		// Dorme e quando acordado irá "pular" uma iteração (continue)
    		raise(SIGSTOP);
    		continue;
    	}
    	// Printa os pids
    	printf("Analista acordado.\n");
    	fclose(lng);
    	print_pids(10);
    	sem_post(sem_block);
    	// No final dorme para ser acordado novamente posteiriormente
    	raise(SIGSTOP);
    }
}