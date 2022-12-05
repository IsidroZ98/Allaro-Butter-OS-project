#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>

sem_t mutex, chair, waitingCustomer;
int emptyChair = 5;
int customerID = 0;
int takenCustomer = 0;
int nextCustomer(){
	customerID++;
	printf("Customer Has Entered: %d\n", customerID);
	return customerID;
}
void takeChair(int n){
	takenCustomer++;
	printf("Customer has taken chair: %d\n", n);	
}
void takeCustomer(){
	printf("Current Passed Customers: %d\n", customerID);
	printf("Barber Has taken Customer\n");
}

void* customer(){
	while(customerID < 20){
			//Critical Section
		int customer = nextCustomer();
		if(emptyChair == 0){
			printf("Customer %d has left\n", customer);
			continue;
		}
		sem_wait(&chair);
		sem_wait(&mutex);
		emptyChair--;
		takeChair(customer);
		sem_post(&mutex);
		sem_post(&waitingCustomer);
	}
}


void* barber(){
	while(takenCustomer < 10){
	//Critical Section
                sem_wait(&waitingCustomer);
		sem_wait(&mutex);
		emptyChair++;
		takeCustomer();
		sem_post(&mutex);
		sem_post(&chair);
        }
}





int main(int argc, char *argv[]){

        pthread_t c, b;

	sem_init(&mutex, 0, 1);
	sem_init(&chair, 0, 5);
	sem_init(&waitingCustomer, 0, 5);
	
	//for(int i = 0; i < 5; i++){
		pthread_create(&c, NULL, customer, NULL);
	//}
	pthread_create(&b, NULL, barber, NULL);

	/*for(int i = 0; i < 5; i++){
                pthread_join(c[i], 0);
        }*/
	pthread_join(c, 0);
        pthread_join(b, 0);

        /*for (int i = 0; i < 5; i++){
                pthread_create(&consumer[i], NULL, consumer_function, NULL);
                pthread_create(&producer[i],
			       	NULL, producer_function, NULL);

        }
        for (int i = 0; i < 5; i++){
                pthread_join(consumer[i], 0);
                pthread_join(producer[i], 0);
        }*/
        return 0;
}
