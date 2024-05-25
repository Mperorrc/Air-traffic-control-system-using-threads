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



int main(){

	Message message;

	key_t key;

	

	key = ftok("airtrafficcontroller.c", 'A');

	if (key == -1){

		fprintf(stderr,"error in creating atc message queue key\n");

		return 1;

	}

	int aid;

	aid = msgget(key, 0644);    

	if (aid == -1){

		fprintf(stderr,"Error in creating atc message queue\n");

		return 1;

	}

	

	int run=1;

	while(run){

		char ipt;

		printf("Do you want the Air Traffic Control System to terminate?(Y for Yes and N for No)\n");

		scanf("%s",&ipt);

		if(ipt=='Y'||ipt=='y'){

			run=0;

		}

		else if(ipt!='N'&&ipt=='n'){

			printf("Invalid input\n");

		}

	}

	message.msg_type=1;

	message.plane_id=0;

	message.arrival_num=0;

	message.dept_num=0;

	message.weight=0;

	message.closes=0;

	message.sender=1;

	if(msgsnd(aid, (void *)&message, sizeof(message), 0) == -1){

		fprintf(stderr,"Error in sending the termination message.\n");

		return 1;

	}

	else{

		printf("Termination request sent.\n");

	}



}

