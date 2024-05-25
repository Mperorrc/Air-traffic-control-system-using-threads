#include <stdio.h>

#include <pthread.h>

#include <semaphore.h>

#include <math.h>

#include <stdlib.h> 

#include <unistd.h>

#include <string.h>

#include <sys/types.h>

#include <sys/ipc.h>

#include <sys/msg.h>



#define MAXLEN 1024



typedef struct atc_msgs{

    long msg_type;

    int plane_id;

    int arrival_num;

    int dept_num;

    int weight;

    int closes;

    int sender;

}Message;



int aid;

int id;

int msg_type;

int loads[11];

sem_t runway_locks[11];



void* thread_func(void *param){

	Message* message = (Message *)param;

	

	int runway_idx = 0;

	while(loads[runway_idx]<message->weight){

		runway_idx++;

	}

	sleep(3);

	printf("Boarding Done\n");

	

	sem_wait(&runway_locks[runway_idx]);

	printf("Plane %d enters runway %d (taking off)\n",message->plane_id,runway_idx+1);

	sleep(2);

	printf("Plane %d exits runway %d\n",message->plane_id,runway_idx+1);

	sem_post(&runway_locks[runway_idx]);

	

	printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n",message->plane_id,runway_idx+1,message->dept_num);

	

	message->sender=id+1;

	message->msg_type = 1;

	message->closes=1;

	if(msgsnd(aid, (void *)message, sizeof(Message), 0) == -1){

		fprintf(stderr, "Error in sending the plane details to ATC.\n");

		return NULL;

	}

}



void* print_func(void *param){

	Message* message = (Message *)param;

	sleep(30);

	//printf("%d,%d,%d,%d,%d,%d\n",message->plane_id,message->arrival_num,message->dept_num,message->weight,message->closes,message->sender);

	int runway_idx=0;

	while(loads[runway_idx]<message->weight){

		runway_idx++;

	}

	sem_wait(&runway_locks[runway_idx]);

	printf("Plane %d enters runway %d (landing)\n",message->plane_id,runway_idx+1);

	sleep(2);

	printf("Plane %d exits runway %d\n",message->plane_id,runway_idx+1);

	sem_post(&runway_locks[runway_idx]);

	sleep(3);

	printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n",message->plane_id,runway_idx+1,message->arrival_num);

	message->sender=id+1;

	message->msg_type = 1;

	message->closes=2;

	if(msgsnd(aid, (void *)message, sizeof(Message), 0) == -1){

		fprintf(stderr, "Error in sending the plane details to ATC.\n");

		return NULL;

	}

}





int main(){

	key_t key;

	

	key = ftok("airtrafficcontroller.c", 'A');

	if (key == -1){

		fprintf(stderr,"error in creating atc message queue key\n");

		return 1;

	}

	

	aid = msgget(key, 0644);    

	if (aid == -1){

		fprintf(stderr,"Error in creating atc message queue\n");

		return 1;

	}

	Message message;

	int run=1;

	printf("Enter Airport Number:\n");

	scanf("%d",&id);

	if(id<=0||id>10){

		fprintf(stderr,"Invalid airport Number.\n");

		return 1;

	}

	

	int runway_cnt;

	

	printf("Enter number of Runways:\n");

	scanf("%d",&runway_cnt);

	if(runway_cnt<=0||runway_cnt>10){

		fprintf(stderr,"Invalid airport Number.\n");

		return 1;

	}

	printf("Enter loadCapacity of Runways (give as a space separated list in a single line):\n");

	for(int i=0;i<runway_cnt;i++){

		scanf("%d",&loads[i]);

		if(loads[i]<1000||loads[i]>12000){

			fprintf(stderr,"Invalid Load Capactiy\n");

			return 1;

		}

	}

	loads[runway_cnt]=15000;

	

	for(int i=0;i<=runway_cnt;i++){

		sem_init(&runway_locks[i], 0, 1);

	}

	

	Message start_msg;

	start_msg.arrival_num=-1;

	start_msg.plane_id = id;

	start_msg.msg_type=1;

	start_msg.sender = id+1;

	if(msgsnd(aid, (void *)&start_msg, sizeof(start_msg), 0) == -1){

		fprintf(stderr, "Error in sending the plane details to ATC.\n");

	}

	

	

	pthread_t tids[1000];

	pthread_attr_t attr;

	pthread_attr_init(&attr);

	int sz=0;

	msg_type=id+1;

	int planes=0;

	int running=1;

	while((running||planes)&&run++){

		Message *received_message = (Message *)malloc(sizeof(Message));



		if (msgrcv(aid, (void *)received_message, sizeof(Message), msg_type, 0) != -1) {

		    if(received_message->closes==1&&received_message->weight==0){

			running=0;

			printf("Closing Message received\n");	

		    }

		    else if(received_message->arrival_num==-1&&received_message->dept_num==-1){

		    	planes++;

		    }

		    else if(received_message->arrival_num==-2&&received_message->dept_num==-2){

		    	planes--;

		    }

		    else if(received_message->closes==1){

			Message *thread_message = (Message *)malloc(sizeof(Message));

			memcpy(thread_message, received_message, sizeof(Message));



			pthread_create(&tids[sz], &attr, print_func, (void *)thread_message);

			sz++;

		    }

		    else{	

			planes++;

			Message *thread_message = (Message *)malloc(sizeof(Message));

			memcpy(thread_message, received_message, sizeof(Message));



			pthread_create(&tids[sz], &attr, thread_func, (void *)thread_message);

			sz++;

		    }

		}



		free(received_message);

	}

	Message end_msg;

	end_msg.arrival_num=-2;

	end_msg.plane_id = id;

	end_msg.msg_type=1;

	end_msg.sender = id+1;

	if(msgsnd(aid, (void *)&end_msg, sizeof(Message), 0) == -1){

		fprintf(stderr, "Error in sending the plane details to ATC.\n");

	}

	

	

}
