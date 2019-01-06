#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#define shmkey 12345
#define keysem 54321

typedef struct{
    int num;
    int temps1;
    int temps2;
    int temps3;
    int tt;//temps total
    int pit;//pit stop
    int out;//arrêt
    pid_t pid;
} voiture;

voiture *voituresCourse;
voiture copieAffichage[20];


void courseTour(int km) {
    int numVoiture[] = {44,77,5,7,3,33,11,31,18,35,27,55,10,28,8,20,2,14,9,16};
    int i;
    for(i = 0; i <= 19; i++){
        int f = fork();
        
        if( f == -1){
            perror("Impossible de créer un fils: Erreur fork.\n");
            exit(-1);
        }
        
        if(f == 0){
            srand(time(0) + getpid());
            int shmid2 = shmget(shmkey, sizeof(voiture), IPC_CREAT | 0666);
            
            if(shmid2 == -1){
                perror("Création de segment impossible: Erreur shmget du fils.\n");
                exit(-1);
            }
            
            voituresCourse = (voiture *)shmat(shmid2, 0, 0);
            if(voituresCourse == (voiture*)-1){
                perror("Attachement impossible: Erreur shmat du fils.\n");
                exit(-1);
            }
            
            /* Sémaphores */
            
            int nsems = 1;
            int semflg = 0;
            int semid;
            
            semid = semget(keysem, nsems, semflg);
            
            // opérations à réaliser atomiquement
            struct sembuf *ops;

            /* Operation WAIT ou P */
            ops[1].sem_num= 0;
            ops[1].sem_op = -1;
            ops[1].sem_flg = SEM_UNDO;
            
            if(semop(semid, ops, 0) == -1){
                perror("erreur de semop");
                exit(EXIT_FAILURE);
            };
            
            int j;
            for(j = 0; j < km; j++){
                int a = rand()%(3300 - 2100) + 2100;
                int b = rand()%(3300 - 2100) + 2100;
                int c = rand()%(3300 - 2100) + 2100;
                int d = a + b + c;
                if(voituresCourse[i].tt == 0){
                    voituresCourse[i].num = numVoiture[i];
                    voituresCourse[i].temps1 = a;
                    voituresCourse[i].temps2 = b;
                    voituresCourse[i].temps3 = c;
                    voituresCourse[i].tt = d;
                    voituresCourse[i].pid = getpid();
                }
                else{
                    if(d < voituresCourse[i].tt){
                        voituresCourse[i].num = numVoiture[i];
                        voituresCourse[i].temps1 = a;
                        voituresCourse[i].temps2 = b;
                        voituresCourse[i].temps3 = c;
                        voituresCourse[i].tt = d;
                        voituresCourse[i].pid = getpid();
                    }
                }
                sleep(1);
            }
            
            /* Opération Signal ou V */
            ops[0].sem_num = 0;
            ops[0].sem_op = +1;
            ops[0].sem_flg = SEM_UNDO;
            
            semop(semid, ops, 0);
            
            
            //il est temps pour le fils de mourrir
            //mais avant il doit se détacher de la memoire partagee
            if(shmdt(voituresCourse) == -1){
                perror("détachement impossible: Erreur shmdt du fils.\n");
                exit(-1);
            }
            //le fils se suicide
            exit(1);
        }
    }
}


void initCourse(){
    int numVoiture[] = {44,77,5,7,3,33,11,31,18,35,27,55,10,28,8,20,2,14,9,16};
    int i;
    for(i = 0; i <= 19; i++){
        voituresCourse[i].num = numVoiture[i];
        voituresCourse[i].temps1 = 0;
        voituresCourse[i].temps2 = 0;
        voituresCourse[i].temps3 = 0;
        voituresCourse[i].tt = 0;
        voituresCourse[i].pid = 0;
    }
}

void trierTab(voiture voitureCopie[20]){
    int i;
    for(i = 0; i <= 19; i++){
        int j;
        for(j = 0; j <=18; j++){
            if(voitureCopie[j].tt > voitureCopie[j+1].tt){
                voiture voit = voitureCopie[j];
                voitureCopie[j]= voitureCopie[j+1];
                voitureCopie[j+1] = voit;
            }
        }
    }
}

int trouverVoitTemps1(voiture voitureCopie[20], int temps){
    int i;
    for(i = 0; i <= 19; i++){
        if(voitureCopie[i].temps1 == temps){
            return voitureCopie[i].num;
        }
    }
    return 0;
}

int trouverVoitTemps2(voiture voitureCopie[20], int temps){
    int i;
    for(i = 0; i <= 19; i++){
        if(voitureCopie[i].temps2 == temps){
            return voitureCopie[i].num;
        }
    }
    return 0;
}

int trouverVoitTemps3(voiture voitureCopie[20], int temps){
    int i;
    for(i = 0; i <= 19; i++){
        if(voitureCopie[i].temps1 == temps){
            return voitureCopie[i].num;
        }
    }
    return 0;
}

int trouverVoitTt(voiture voitureCopie[20], int temps){
    int i;
    for(i = 0; i <= 19; i++){
        if(voitureCopie[i].tt == temps){
            return voitureCopie[i].num;
        }
    }
    return 0;
}

int maxS1(voiture voitureCopie[20]){
    int i;
    int max = 0;
    for(i = 0; i <= 19; i++){
        if(voitureCopie[i].temps1 > max){
            max = voitureCopie[i].temps1;
        }
    }
    return trouverVoitTemps1(voitureCopie, max);
}

int maxS2(voiture voitureCopie[20]){
    int i;
    int max = 0;
    for(i = 0; i <= 19; i++){
        if(voitureCopie[i].temps2 > max){
            max = voitureCopie[i].temps2;
        }
    }
    return trouverVoitTemps2(voitureCopie, max);
}

int maxS3(voiture voitureCopie[20]){
    int i;
    int max = 0;
    for(i = 0; i <= 19; i++){
        if(voitureCopie[i].temps3 > max){
            max = voitureCopie[i].temps3;
        }
    }
    return trouverVoitTemps3(voitureCopie, max);
}

int maxCourse(voiture *voitureCopie){
    int i;
    int max = 0;
    for(i = 0; i <= 19; i++){
        if(voitureCopie[i].tt > max){
            max = voitureCopie[i].tt;
        }
    }
    return trouverVoitTt(voitureCopie, max);
}

void affichage(char str1[]){
    system("clear");
    printf("%s", str1);
    printf("\t|\tS1\t\t|\tS2\t\t|\tS3\t\t|\tTT\t\t|\tPIT\t|\tOUT\t\n\n");
    voiture voitureCopie[20];
    
    /** Sémaphores **/
    int nblecteur = 0;
    
    /* Semaphores */
    
    int nsems = 1;
    int semflg = 0;
    int semid;
    
    // opérations à réaliser atomiquement
    struct sembuf *ops;
    
    semid = semget(keysem, nsems, semflg);
    semctl(semid, 0, SETVAL, 1);
    semctl(semid, 1, SETVAL, 1);
    
    /* P(MUTEX */
    ops[1].sem_num= 1;
    ops[1].sem_op = -1;
    ops[1].sem_flg = SEM_UNDO;
    
    semop(semid, ops, 0);
    
    nblecteur = nblecteur + 1;
    
    if(nblecteur == 1){
        /* P( ACCESS ) */
        ops[1].sem_num= 0;
        ops[1].sem_op = -1;
        ops[1].sem_flg = SEM_UNDO;
        
        semop(semid, ops, 0);
    }
    else{
        /* V(Mutex) */
        ops[1].sem_num = 1;
        ops[1].sem_op = -1;
        ops[1].sem_flg = SEM_UNDO;
        
        semop(semid, ops, 0);
    }
    
    memcpy(&voitureCopie, voituresCourse, 20*sizeof(voiture)); // Section critique
    
    nblecteur = nblecteur - 1;
    
    if(nblecteur == 0){
        /* V(ACCESS) */
        ops[1].sem_num = 0;
        ops[1].sem_op = +1;
        ops[1].sem_flg = SEM_UNDO;
        
        semop(semid, ops, 0);
    }
    
    /* V(MUTEX ) */
    
    ops[1].sem_num = 1;
    ops[1].sem_op = +1;
    ops[1].sem_flg = SEM_UNDO;
    
    semop(semid, ops, 1);
    
    trierTab(voitureCopie);
    int i;
    for(i = 0; i <= 19; i++){
        int div1 = voitureCopie[i].temps1 / 60;
        int res1 = voitureCopie[i].temps1 % 60;
        
        int div2 = voitureCopie[i].temps2 / 60;
        int res2 = voitureCopie[i].temps2 % 60;
        
        int div3 = voitureCopie[i].temps3 / 60;
        int res3 = voitureCopie[i].temps3 % 60;
        
        int div = voitureCopie[i].tt /3600;
        int res = (voitureCopie[i].tt % 3600) / 60;
        int mn = (voitureCopie[i].tt % 3600) % 60;
        
        printf("%d\t|\t00:%d:%d m\t|\t00:%d:%d m\t|\t00:%d:%d m\t|\t0%d:%d:%d m\t|\t(P)\t|\tIN\t\n", voituresCourse[i].num, div1, res1, div2, res2, div3, res3, div, res, mn);
        
    }
    printf("\nMeilleur S1: %d\n", maxS1(voitureCopie));
    printf("Meilleur S2: %d\n", maxS2(voitureCopie));
    printf("Meilleur S3: %d\n", maxS3(voitureCopie));
    printf("Meilleur de la course en elle-même: %d\n", maxCourse(voitureCopie));
    sleep(1);
}

void courseSession(char str1[], int trs, int km){
    int i;
    for(i= 0; i<trs; i++){
        courseTour(km);
        affichage(str1);
    }
}

int main(int argc, char *argv[]){
    int shmid = shmget(shmkey, sizeof(voiture), IPC_CREAT | 0666);
    if(shmid == -1){
        perror("Création de segment impossible: Erreur shmget du père.\n");
        exit(-1);
    }
    
    voituresCourse = (voiture *)shmat(shmid, 0, 0);
    if(voituresCourse == (voiture*)-1){
        perror("Attachement impossible: Erreur shmat du pére.\n");
        exit(-1);
    }
    
    initCourse();
    courseSession("COURSE P1 : Premiere scéance d'essais !!!\n\n\n", 50, 100);
    printf("%s\n", "\n\nPremière course terminee\n\n\n" );
    sleep(1);
    initCourse();
    courseSession("COURSE P2 : Deuxième scéance d'essais !!!\n\n\n", 30, 50);
    printf("%s\n", "\n\nDeuxieme course terminee\n\n\n" );
    sleep(1);
    initCourse();
    courseSession("COURSE P3 : Troisième scéance d'essais !!!\n\n\n", 30, 50);
    printf("%s\n", "\n\nTroisième course terminee\n\n\n" );
    sleep(1);
    initCourse();
    
    if(shmdt(voituresCourse) == -1){
        perror("détachement impossible: Erreur shmdt du père.\n");
        exit(-1);
    }
    
    if(shmctl(shmid, IPC_RMID,  (struct shmid_ds *) NULL) == -1){
        perror("destruction de la memoire partagee n'est pas faite: Erreur shmctl.\n");
        exit(-1);
    }
    exit(1);
    //le père attend la mort du fils
    wait(0);
    
    return 1;
}
