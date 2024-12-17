#include <iostream>
#include <vector>

//Multi-Process Handling
#include <pthread.h>

//Process 
#include <csignal>
#include <unistd.h>

//Semaphore
#include <semaphore.h>
#include <fcntl.h>

//Time
#include <sys/time.h>
// Client struct
struct Client{
	pid_t pid;
	// Arrive time in u seconds (10^{-6})
	timeval arrive,exit;
	int priority;
	// wait time for service
	double wTime;
};
sem_t* sem_atend;
sem_t* sem_block;
Client* normalClients;
Client* priorityClients;
int nProcesses;
FILE* lng;

int randomPriority (){
	return (rand()%2);
}

void *stop (void* _){
	int i = 0;
	while(getchar()!='s'){
		std::cout << i;
	}
	exit(0);
	return nullptr;
}
void *service (void* arg){
	while (1) {
		if (normalClients==NULL){
			
		}
	}
	FILE* demanda = fopen("demanda.txt","r");
	fclose(demanda);
	return nullptr;
}
void *reception (void *arguments){
	sem_atend = sem_open("/sem_atend", O_CREAT | O_EXCL, 0644, 1);
	if (sem_atend == SEM_FAILED) {
		sem_unlink("/sem_atend");
		sem_atend = sem_open("/sem_atend", O_CREAT | O_EXCL, 0644, 1);
	}
	sem_block = sem_open("/sem_block", O_CREAT | O_EXCL, 0644, 1);
	if (sem_block == SEM_FAILED) {
		sem_unlink("/sem_block");
		sem_block = sem_open("/sem_block", O_CREAT | O_EXCL, 0644, 1);
	}
	std::string nProcessesString= (nProcesses == 0) ? "Infinite" : std::to_string(nProcesses);
	std::cout << "Processes: " << nProcessesString << '\n';
	
	if (nProcesses == 0){
		normalClients = new Client[100];
		priorityClients = new Client[100];
		int counter = 1;
		while (1){
			Client aux;
			gettimeofday(&aux.arrive,NULL);
			//std::cout << "Cliente "<< counter << " criado." << '\n';

			counter++;
		}
	}else{
		normalClients = new Client[nProcesses];
		priorityClients = new Client[nProcesses];
		int counter = 1;
		while (counter <= nProcesses){
			Client aux;
			gettimeofday(&aux.arrive,NULL);
			//std::cout << "Cliente "<< counter << " criado." << '\n';
			counter++;
		}
	}
	exit(0);
	return nullptr;
}


int main (int narg,char *argv[]){
	if (narg >= 2)
		nProcesses = atoi(argv[1]);
	else
		nProcesses = 0;
	sem_close(sem_atend);
	sem_unlink("/sem_atend");
	sem_close(sem_block);
	sem_unlink("/sem_block");
	int numCPU = 		sysconf(_SC_NPROCESSORS_ONLN);
	std::cout << "Avaliable Cores:  " << numCPU << '\n';
	lng = fopen("lng.txt","w+");
	pthread_t thread1,stopThread,serviceThread1,scheduler;
	pthread_create(&serviceThread1,NULL,service,NULL);
	pthread_create (&thread1,NULL,reception,NULL);
	pthread_create (&stopThread,NULL,stop,NULL);
	pthread_join(stopThread,NULL);
	pthread_join(thread1,NULL);
	pthread_join(serviceThread1,NULL);
	return 0;
}