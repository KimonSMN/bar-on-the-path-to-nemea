#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <string.h>
int main(int argc, char* argv[]){

  //////// HANDLE COMMAND LINE ARGUMENTS ////////
    int resttime;   // παρέχει την μέγιστη δυνατή περίοδο στην οποία ο επισκέπτης παραμένει στο τραπέζι αφότου έχει εξυπηρετηθεί από τον υπεύθυνο (σε κατάσταση sleep).
    int shmid;      // δίνει το κλειδί που το κοινό τμήμα μνήμης έχει (και όπου υπάρχουν αντικείμενα κοινού ενδιαφέροντος).

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            resttime = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0) {
            shmid = atoi(argv[++i]);
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }

    if(argc == 1){
        printf("Please provide -d & -s flags\n");
        return 0;
    }

    printf("resttime: %d\n", resttime);
    printf("shmid: %d\n", shmid);

    // Check if there is empty chair at table
}