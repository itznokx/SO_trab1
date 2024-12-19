#include "analista.h"

sem_t* sem_block;

// 1024 buffer size
#define ALLOC_SIZE 1024
#define MAX_PRINT 10
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
int print_pids(int PID_MAX_PRINT){
	FILE* lng = fopen("lng.txt","r+");
	int pids[MAX_PRINT];
    int count = 0;
	while(count < PID_MAX_PRINT && fscanf(lng, "%d", &pids[count])==1) {
    	printf("PID: %i\n",pids[count]);
        count++;
    }
    return count;
}
int main() {
	start_analist();
    sem_block = sem_open("/sem_block", O_RDWR);
    if (sem_block == SEM_FAILED) {
        sem_block = sem_open("/sem_block", O_CREAT, 0644, 1);
    }
    raise(SIGSTOP);
    while(1) {
        sem_wait(sem_block);
        // Ler até 10 pids do arquivo lng.txt
        FILE *lng = fopen("lng.txt", "r+");
        if (!lng) {
            // se não existe, desbloqueia e dorme
            sem_post(sem_block);
            raise(SIGSTOP);
            continue;
        }
        fclose(lng);
        int count = print_pids(10);
        if (count == 0) {
            fclose(lng);
            sem_post(sem_block);
            raise(SIGSTOP);
            continue;
        }
        fseek(lng, 0, SEEK_SET);
        int pidsArray[ALLOC_SIZE];
        int total = 0;
        fclose(lng);
        lng = fopen("lng.txt","r");
        int aux;
        int skip = count;
        while(fscanf(lng,"%d",&aux)==1) {
            if(skip>0) {
                skip--;
                continue;
            }
            pidsArray[total++]=aux;
        }
        fclose(lng);

        // Reescrever o arquivo sem os primeiros 10
        lng = fopen("lng.txt","w");
        for (int i=0; i<total; i++) {
            fprintf(lng,"%d\n",pidsArray[i]);
        }
        fclose(lng);

        sem_post(sem_block);
        // Volta a dormir
        raise(SIGSTOP);
    }
    return 0;
}