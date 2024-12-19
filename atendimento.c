#include "atendimento.h"

#define true 	1
#define false 	0
#define nullptr NULL

// Variaveis globais

int 	nProcesses ;
int 	globalPatience;
int 	running=1;
int 	satisfieds =0;
int 	totalClients = 0;
int 	MAX_QUEUE_CLIENTS = 100;
pthread_t receptionThread,serviceThread1,stopThread;
// Semáforos

sem_t* sem_block;
sem_t* sem_atend;
sem_t* sem_nQueue;
sem_t* sem_pQueue;
// Mutex QUEUE
pthread_mutex_t nQueue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pQueue_mutex = PTHREAD_MUTEX_INITIALIZER;
// Condição de fila não cheia -> dequeue function use
pthread_cond_t nQueue_not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t pQueue_not_full = PTHREAD_COND_INITIALIZER; 
// Condição de fila não vazia -> enqueue function use
pthread_cond_t nQueue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t pQueue_not_empty = PTHREAD_COND_INITIALIZER;

// PID do analista

pid_t analistaPID;

typedef struct ArgsPass
{
	FilaCliente* fila1;
	FilaCliente* fila2;
} ArgsPass;


int randomPriority (){
	return (rand()%2);
}
void start_queue(FilaCliente *queue,int alloc_size,int priority){
	//queue = (FilaCliente*)malloc(sizeof(FilaCliente));
	queue->first = (Cliente*)malloc(alloc_size*sizeof(FilaCliente));
	// Bloqueador de fila
	queue->size = 0;
	queue->max_size = alloc_size;
	queue->priority = priority;
	queue->first = NULL;
	queue->last = NULL;
}
void destroy_queue(FilaCliente* queue){
	free(queue->first);
	free(queue->last);
}
Cliente* new_Client(pid_t pid,int serviceTime,int pp){
	Cliente* newClient = (Cliente*)malloc(sizeof(Cliente));
	gettimeofday(&newClient->arrive,NULL);
	newClient->pid = pid;
	newClient->serviceTime = serviceTime;
	newClient->priority = pp;
	newClient->patience = (pp==0)?globalPatience:globalPatience/2;
	newClient->next = NULL;
	return newClient;
}
void enqueue (FilaCliente *queue,Cliente *client){
	// Bloqueia a fila
	if (queue->priority==0)
		pthread_mutex_lock (&nQueue_mutex);
	else
		pthread_mutex_lock (&pQueue_mutex);
	// Verifica se a fila está cheia
	while (queue->size == MAX_QUEUE_CLIENTS){
		if (queue->priority==0)
		pthread_cond_wait (&nQueue_not_full,&nQueue_mutex);
	else
		pthread_cond_wait (&pQueue_not_full,&pQueue_mutex);
	}
	if (queue->first==NULL){
		queue->first = client;
		queue->last = client;
		queue->last->next = NULL;
	}
	else{
		queue->last->next = client;
		queue->last= client;
	}
		queue->size++;
	if (queue->priority==0){
		pthread_cond_signal (&nQueue_not_empty);
		pthread_mutex_unlock(&nQueue_mutex);
	}
	else{
		pthread_cond_signal (&pQueue_not_empty);
		pthread_mutex_unlock(&pQueue_mutex);
	}
}
Cliente* dequeue(FilaCliente *queue){
	if (queue->priority==0)
		pthread_mutex_lock (&nQueue_mutex);
	else
		pthread_mutex_lock (&pQueue_mutex);
	while (queue->size == 0){
		if (queue->priority==0){
			pthread_cond_wait (&nQueue_not_empty,&nQueue_mutex);
		}
		else{
			pthread_cond_wait (&pQueue_not_empty,&pQueue_mutex);
		}
	}
	Cliente *client = queue->first;
	queue->first = client->next;
	queue->size--;
	if (queue->priority==0){
		pthread_cond_signal (&nQueue_not_empty);
		pthread_mutex_unlock(&nQueue_mutex);
	}
	else{
		pthread_cond_signal (&pQueue_not_empty);
		pthread_mutex_unlock(&pQueue_mutex);
	}

	return client;
}
// Thread-function para parar o programa
void* stop_program(void* args){
	char c;
	printf("Stop_program is listening. (s to stop)\n");
	while ((running==1)&&(getchar()!='s')){
		if (c=='s'){
			running=0;
			break;
		}
	}
	running=0;
	return NULL;
}
int start_analist (){
	int aux;
	pid_t pidAnalist = fork();
	if (pidAnalist == 0){
		execl("./analista.out","empty",(char*)NULL);
		exit(1);
	}
	// Setar PID global do analista;
	FILE* analist = fopen("pidanalista.txt","r");
	if (!analist){
		printf("Analist Started.\n");\
		return pidAnalist;
	}
	fscanf(analist,"%i",&aux);
	return aux;
}

void wake_analist(){
	kill(analistaPID,SIGCONT);
}

long calculate_time_difference (struct timeval start, 
								struct timeval end){
	long seconds = end.tv_sec - start.tv_sec;
    long useconds = end.tv_usec - start.tv_usec;
    return seconds*1000 + useconds/1000;
}
void calculate_program_time(struct timeval start,
							struct timeval end){
	long total_time = calculate_time_difference(start,end);
	printf("Tempo total de execução: %ld ms\n", total_time);
}
void calculate_satisfaction(){
	kill(analistaPID, SIGKILL);
	sem_close(sem_atend);
    sem_unlink("/sem_atend");
    sem_close(sem_block);
    sem_unlink("/sem_block");
    double satisfaction_rate = 0;
    if (totalClients>0) {
        satisfaction_rate = (double)satisfieds / (double)totalClients;
    }
    printf("Taxa de satisfação: %.2f%%\n", satisfaction_rate*100);
    
}

void* reception(void* args){
	/*
	printf("Reception created.\n");
	if (nProcesses==0){
    	printf("Processes: Infinite.\n");
	}else{
		printf("Processes: %d\n",nProcesses);
	}
	*/
	ArgsPass* argStruct = (ArgsPass*)args;
	FilaCliente* normalQueue 	= (FilaCliente*)argStruct->fila1;
	FilaCliente* priorityQueue 	= (FilaCliente*)argStruct->fila2;
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
    analistaPID = start_analist();
    int created = 0;
    while ((running&&nProcesses==0)|| (nProcesses>0&&created<nProcesses)){
    	if (running==0){
    		break;
    	}
    	while (nProcesses==0 && !running){
    		break;
    	}
    	if (nProcesses>0 && created>nProcesses){
    		break;
    	}
    	int pAux = randomPriority();
    	if (pAux==0){
    		pthread_mutex_lock(&nQueue_mutex);
    		// Verificar se a fila estiver cheia
    		while(normalQueue->size==MAX_QUEUE_CLIENTS && 
        		(nProcesses==0 || (nProcesses>0 && created<nProcesses))) {
        	// Se a fila estiver cheia 
	            pthread_cond_wait(&nQueue_not_full,&nQueue_mutex);
	        	// Desbloqueia o cadeado da fila
        	}
	        pthread_mutex_unlock(&nQueue_mutex);
        }
        else {
        	pthread_mutex_lock((&pQueue_mutex));
    		// Verificar se a fila estiver cheia
    		while(priorityQueue->size==MAX_QUEUE_CLIENTS && (nProcesses==0 || (nProcesses>0 && created<nProcesses)))
    		{
	        	// Se a fila estiver cheia 
	            pthread_cond_wait(&pQueue_not_full,&pQueue_mutex);
	        	// Desbloqueia o cadeado da fila
       		}
	        pthread_mutex_unlock(&pQueue_mutex);
    	}
       	if (nProcesses>0 && created>=nProcesses) 
       		break;
        if (nProcesses==0 && !running) 
        	break;
        pid_t pid = fork();
        if (pid < 0){
        	perror("fork error.");
        	exit(1);
        }
        if (pid==0){
        	execl("./cliente.out","empty",(char*)NULL);
            perror("execl error.");
            exit(1);
        }
   		int serviceTime;
        FILE *f = fopen("demanda.txt","r+");
        if (!f){
        	perror("demanda");
        	serviceTime = 15;
        }else{
        	fscanf(f,"%d",&serviceTime);
        	fclose(f);
        }
        Cliente* client = new_Client(pid,serviceTime,pAux);
        gettimeofday(&client->arrive,NULL);
        if (pAux==0)
        	enqueue(normalQueue,client);
        else
        	enqueue(priorityQueue,client);
        //printf("Cliente %d criado. (%d) (%d)\n",created,client->serviceTime,client->priority);
        created++;
    }
    printf("cabou reception\n");
    return NULL;
}
void* service(void* args){
	ArgsPass* argStruct = (ArgsPass*)args;
	FilaCliente* normalQueue 	= argStruct->fila1;
	FilaCliente* priorityQueue 	= argStruct->fila2;
	int counter = 0;
	while (	(nProcesses&&totalClients<nProcesses)||
			(nProcesses==0&&running)||
			(nProcesses==0&&normalQueue->size>0)||
			(nProcesses==0&&priorityQueue->size>0)){
		if ((nProcesses==0) && (!running) && 
			(normalQueue->size)==0 && 
			(priorityQueue->size)==0){
			break;
		}
		if (running==0)
			break;
		Cliente* client;
		// Atende 1 a cada 3 da fila Normal
		int cond = counter%3;
		if ((cond==0)){
			if (normalQueue->size > 0){
				client = dequeue(normalQueue);
				//printf("Dequeue normal\n");
				//printf("queue size: %i\n",normalQueue->size);
			}else{
				counter++;
				continue;
			}
		}
		else{
			if (priorityQueue->size > 0){
				client = dequeue(priorityQueue);
				//printf("Dequeue priority\n");
				//printf("queue size: %i\n",priorityQueue->size);
			}else{
				counter++;
				continue;
			}
		}
		kill (client->pid,SIGCONT);
		//printf("(%d) PID-> %d\n",client->priority,client->pid);
		sem_wait(sem_atend);
		struct timeval end_service;
		gettimeofday(&end_service,NULL);
		long time_passed = calculate_time_difference(client->arrive,end_service);
		long patience = client->patience;
		counter++;
		totalClients++;
		//printf("totalClients: %d\n",totalClients);
		if (time_passed<=patience)
			satisfieds++;
		sem_wait(sem_block);
		FILE *lng = fopen("lng.txt","a+");
		fprintf(lng, "%d\n",client->pid);
		fclose(lng);
		sem_post(sem_block);
		sem_post(sem_atend);
		//printf("checkpoint3\n");
		if (totalClients>0 && totalClients%10==0) wake_analist();
	}
	printf("Cabou service.\n");
	running = false;
	return NULL;
}
void clean(){
	remove ("lng.txt");
	remove ("demanda.txt");
}
int main (int narg,char* argv[]){
	if (narg<3){
		printf("Formato invalido. Use: ./a.out N P\n");
		printf("N: numero de clientes e P: paciencia.\n");
		return 1;
	}else{
		nProcesses = atoi(argv[1]);
		globalPatience = atoi(argv[2]);
	}
	//clean();
	struct timeval program_start,program_end;
	gettimeofday(&program_start,NULL);
	if (nProcesses>0){
		MAX_QUEUE_CLIENTS=nProcesses;
	}
	printf("MAX_QUEUE_CLIENTS: %d\n",MAX_QUEUE_CLIENTS);
	printf("Client Processes: %d\n",nProcesses);
	printf("Global Patience: %d\n",globalPatience);
	FilaCliente nQueue;
	FilaCliente pQueue;
	start_queue(&nQueue,MAX_QUEUE_CLIENTS,0);
	start_queue(&pQueue,MAX_QUEUE_CLIENTS,1);
	ArgsPass args;
	args.fila1= &nQueue;
	args.fila2= &pQueue;

	pthread_create(&receptionThread,NULL,reception,(void*)&args);
	pthread_create(&serviceThread1,NULL,service,(void*)&args);
	pthread_create(&stopThread,NULL,stop_program,NULL);
	pthread_join(serviceThread1,NULL);
	pthread_join(receptionThread,NULL);
	pthread_join(stopThread,NULL);
	while(running==true){
	}
	gettimeofday(&program_end,NULL);
	printf("End of threads.\n");
	calculate_satisfaction();
	long seconds,useconds;
	seconds = program_end.tv_sec - program_start.tv_sec;
	useconds = program_end.tv_usec - program_start.tv_usec;
	double total_time = seconds + useconds/1000000.0;
	printf("Tempo total de execução: %.3f ms\n", total_time);
	pthread_mutex_destroy(&nQueue_mutex);
	pthread_mutex_destroy(&pQueue_mutex);
    pthread_cond_destroy(&nQueue_not_full);
    pthread_cond_destroy(&pQueue_not_full);
    pthread_cond_destroy(&nQueue_not_empty);
    pthread_cond_destroy(&pQueue_not_empty);
	destroy_queue(&nQueue);
	destroy_queue(&pQueue);
	clean();
	return 0;
}