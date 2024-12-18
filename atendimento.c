#include "atendimento.h"

#define true 	1
#define false 	0
#define nullptr NULL

// Variaveis globais

int 	nProcesses;
int 	globalPatience;
int 	running;
int 	satisfieds;
int 	totalClients;
int 	MAX_QUEUE_CLIENTS;
struct timeval program_start,program_end;
// Semáforos

sem_t* sem_block;
sem_t* sem_atend;

// Filas de Cliente

Queue normalQueue;
Queue priorityQueue;

// PID do analista

pid_t analistaPID;

// Thread Lockers

// Bloqueador de fila
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
// Condição de fila não cheia -> dequeue function use
pthread_cond_t queue_not_full = PTHREAD_COND_INITIALIZER;
// Condição de fila não vazia -> enqueue function use
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
int randomPriority (){
	return (rand()%2);
}
void enqueueNormal (Cliente client){
	pthread_mutex_lock (&queue_mutex);
	while (normalQueue.count == MAX_QUEUE_CLIENTS){
		pthread_cond_wait (&queue_not_full,&queue_mutex);
	}
	normalQueue.queue[normalQueue.size-1] = client;
	normalQueue.count++;
	pthread_cond_signal (&queue_not_empty);
	pthread_mutex_unlock(&queue_mutex);
}
Cliente dequeueNormal (){
	pthread_mutex_lock (&queue_mutex);
	while (normalQueue.count == 0){
		pthread_cond_wait (&queue_not_empty, &queue_mutex);
	}
	Cliente client = normalQueue.queue[normalQueue.front];
	normalQueue.front = (normalQueue.front+1)%MAX_QUEUE_CLIENTS;
	normalQueue.count--;
	pthread_cond_signal(&queue_not_full);
	pthread_mutex_unlock(&queue_mutex);
	return client;
}
void enqueuePrioriry (Cliente client){
	pthread_mutex_lock (&queue_mutex);
	while (priorityQueue.count == MAX_QUEUE_CLIENTS){
		pthread_cond_wait (&queue_not_full,&queue_mutex);
	}
	priorityQueue.queue[priorityQueue.size-1] = client;
	priorityQueue.count++;
	pthread_cond_signal (&queue_not_empty);
	pthread_mutex_unlock(&queue_mutex);
}
Cliente dequeuePriority (){
	pthread_mutex_lock (&queue_mutex);
	while (priorityQueue.count == 0){
		pthread_cond_wait (&queue_not_empty, &queue_mutex);
	}
	Cliente client = priorityQueue.queue[priorityQueue.front];
	priorityQueue.front = (priorityQueue.front+1)%MAX_QUEUE_CLIENTS;
	priorityQueue.count--;
	pthread_cond_signal(&queue_not_full);
	pthread_mutex_unlock(&queue_mutex);
	return client;
}
// Thread-function para parar o programa
void* stop_program(void* args){
	while (getchar()!='s'){

	}
	running = false;
	exit(0);
	return nullptr;
}
int start_analist (){
	pid_t pidAnalist = fork();
	if (pidAnalist == 0){
		execl("./analista.out","empty",(char*)NULL);
		exit(1);
	}
	// Setar PID global do analista;
	analistaPID = pidAnalist; 
	return pidAnalist;
}
void wake_analist(){
	kill(analistaPID,SIGCONT);
}
void calculate_satisfaction(){
	kill(analistaPID, SIGKILL);
	sem_close(sem_atend);
    sem_unlink("/sem_atend");
    sem_close(sem_block);
    sem_unlink("/sem_block");
    if (totalClients>0) {
        satisfaction_rate = (double)satisfieds / (double)totalClients;
    }
    gettimeofday(&program_end,NULL);
    double satisfaction_rate = 0;
    long total_time = calculate_time_difference(program_start,program_end);
    printf("Taxa de satisfação: %.2f%%\n", satisfaction_rate*100);
    printf("Tempo total de execução: %ld ms\n", total_time);
}
long calculate_time_difference (struct timeval arrive, 
								struct timeval timeOfService){
	long seconds = timeOfService.tv_sec - arrive.tv_sec;
    long useconds = timeOfService.tv_usec - arrive.tv_usec;
    return seconds*1000 + useconds/1000;
}
void* reception(void* args){
	sem_atend = sem_open("/sem_atend", O_CREAT|O_EXCL, 0644, 1);
    if (sem_atend == SEM_FAILED) {
        sem_unlink("/sem_atend");
        sem_atend = sem_open("/sem_atend", O_CREAT, 0644, 1);
    }
    sem_block = sem_open("/sem_block", O_CREAT|O_EXCL, 0644, 1);
    if (sem_block == SEM_FAILED) {
        sem_unlink("/sem_block");
        sem_block = sem_open("/sem_block", O_CREAT, 0644, 1);
    }
    int created = 0;
    while ((running&&nProcesses==0)||
    	   (nProcesses>0&&created<nProcesses)){
    	while (nProcesses==0 && !running){
    		break;
    	}
    	if (nProcesses>0 && created>=nProcesses)
    		break;
    	pthread_mutex_lock(&queue_mutex);
    	// Verificar se a fila estiver cheia
        while(	normalQueue.count==MAX_QUEUE && 
        		(nProcesses==0 || (nProcesses>0 && created<N))) {
        	// Se a fila estiver cheia 
            pthread_cond_wait(&queue_not_full, &queue_mutex);
        }
        int pAux = randomPriority();
        // Desbloqueia o cadeado da fila
        pthread_mutex_unlock(&queue_mutex);
        if ((nProcesses>0&&created>=nProcesses)||
        	(nProcesses==0)&& !running){
        	break;
        }

        pid_t pid = fork();
        if (pid < 0){
        	perror("fork error.");
        	exit(1);
        }
        if (pid==0){
        	execl("./cliente","empty",(char*)NULL);
            perror("execl error.");
            exit(1);
        }
        Cliente client;
        client.pid = pid;
        gettimeofday(&client.arrive,NULL);
        
        client.priority = randomPriority();
        FILE *f = fopen("demanda.txt","r");
        if (!f){
        	perror("demanda open error");
        	client.serviceTime = 15;
        }else{
        	fscanf(f,"%d",&client.serviceTime);
        	fclose(f);
        }
        if (pAux==0)
        	enqueueNormal(client);
        else
        	enqueuePrioriry(client);
        created++;
    }
    return nullptr;
}
void* service(void* args){
	while (	(nProcesses&&totalClients<nProcesses)||
			(nProcesses==0&&running)||
			(nProcesses==0&&normalQueue.count>0)||
			(nProcesses==0&&priorityQueue.count>0)){
		if (nProcesses==0 && !running && 
			normalQueue.count==0 && 
			priorityQueue.count==0){
			break
		}
		Cliente client;
		// Atende 1 a cada 3 da fila Normal
		if ((totalClients%3==0)&&normalQueue.count>0){
			client = dequeueNormal();
		}
		if ((totalClients%3!=0)&&priorityQueue.count>0){
			client = priorityQueue();
		}
		kill (client.pid,SIGCONT);
		sem_wait(sem_atend);
		struct timeval end_service;
		gettimeofday(&end_service,NULL);
		long time_passed = calculate_time_difference(client.arrive,end_service);
		sem_wait(sem_block);
		FILE *f = fopen("lng.txt","w+");
		fprintf(f, "%d\n",client.pid);
		fclose(f)
		sem_post(sem_block);
		if ((nProcesses-totalClients<10)||(totalClients%10==0))
			wake_analist();
	}
	return nullptr;
}
void clean(){
	remove ("lng.txt");
	remove ("demanda.txt");
}
int main (int narg,char* argv[]){
	if (argc<3){
		printf(	"Formato invalido. Use: ./a.out N P\n
				N: numero de clientes e P: paciencia.\n");
		return 1;
	}
	gettimeofday(&program_start,NULL);
	nProcesses = atoi(argv[1]);
	X = atoi(argv[2]);
	if (nProcesses==0){
		MAX_QUEUE_CLIENTS=100;
	}
	else {
		MAX_QUEUE_CLIENTS=nProcesses;
	}
	clean();
	pthread_t receptionThread,serviceThread1,stopThread;
	pthread_create(&receptionThread,NULL,reception,NULL);
	pthread_create(&serviceThread1,NULL,service,NULL);
	pthread_create(&stopThread,NULL,stop_program,NULL);
	pthread_join(stopThread,NULL);
	pthread_join(receptionThread,NULL);
	pthread_join(serviceThread1,NULL);
	return 0;
}