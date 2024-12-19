#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>

#define MAX_PRINT 10

static sem_t *sem_block;

void handle_sigcont(int sig) {
    // Ao receber SIGCONT, apenas continuar execução no main
}

int main() {
    signal(SIGCONT, handle_sigcont);

    sem_block = sem_open("/sem_block", O_RDWR);
    if (sem_block == SEM_FAILED) {
        perror("sem_block open");
        exit(1);
    }

    // Inicialmente dorme (SIGSTOP)
    raise(SIGSTOP);

    // Loop infinito: 
    // O analista será acordado pelo processo atendimento via SIGCONT. 
    // Quando acordado:
    //   1. Bloqueia LNG
    //   2. Lê até 10 PIDs
    //   3. Imprime e remove do arquivo
    //   4. Desbloqueia e volta a dormir
    // Se receber EOF ou não houver nada a imprimir, dorme novamente.

    while(1) {
        sem_wait(sem_block);

        // Ler até 10 pids do arquivo lng.txt
        FILE *f = fopen("lng.txt", "r+");
        if (!f) {
            // se não existe, desbloqueia e dorme
            sem_post(sem_block);
            raise(SIGSTOP);
            continue;
        }

        int pids[MAX_PRINT];
        int count = 0;
        // ler no máximo 10 pids
        while(count < MAX_PRINT && fscanf(f, "%d", &pids[count])==1) {
            count++;
        }

        if (count == 0) {
            // Nada a imprimir
            fclose(f);
            sem_post(sem_block);
            raise(SIGSTOP);
            continue;
        }

        // Imprimir
        printf("[Analista] Imprimindo %d PIDs:\n", count);
        for (int i=0; i<count; i++) {
            printf("%d\n", pids[i]);
        }

        // Agora precisamos remover estes 10 (ou menos) pids do arquivo.
        // Vamos ler o resto e reescrever:
        fseek(f, 0, SEEK_SET);
        int all_pids[10000]; // buffer grande por simplicidade
        int total = 0;
        // já lemos count pids, então descartar eles
        // continuar lendo do arquivo
        for (int i=0; i<count; i++) {
            // já foram lidos e impressos
        }
        // Ler o restante
        // Notar que já estamos no final da leitura da primeira vez.
        // Precisamos reabrir o arquivo para ler de novo do início?
        // Melhor: fechar e reabrir
        fclose(f);
        f = fopen("lng.txt","r");
        int dummy;
        int skip = count;
        while(fscanf(f,"%d",&dummy)==1) {
            if(skip>0) {
                skip--;
                continue;
            }
            all_pids[total++]=dummy;
        }
        fclose(f);

        // Reescrever o arquivo sem os primeiros 10
        f = fopen("lng.txt","w");
        for (int i=0; i<total; i++) {
            fprintf(f,"%d\n",all_pids[i]);
        }
        fclose(f);

        sem_post(sem_block);
        // Volta a dormir
        raise(SIGSTOP);
    }

    return 0;
}
