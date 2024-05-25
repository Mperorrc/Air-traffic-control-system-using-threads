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

	

	int num_airports;

	printf("Enter the number of airports to be handled/managed:\n");

	scanf("%d",&num_airports);

	if(num_airports<2||num_airports>10){

		fprintf(stderr,"Invalid airport cnt.\n");

		return 1;

	}

	key_t key;

	

	key = ftok("airtrafficcontroller.c", 'A');

	if (key == -1){

		fprintf(stderr,"error in creating atc message queue key\n");

		return 1;

	}

	

	int aid = msgget(key, 0644|IPC_CREAT);    

	if (aid == -1){

		fprintf(stderr,"Error in creating atc message queue\n");

		return 1;

	}

	int running_cnt = 0;

	int running_airports[11] = {0};

	

	FILE* file;

	file = fopen("AirTrafficController.txt", "a");

	

	Message message;

	/*

		if message type is 1 -> cleanup message to close

		if message type is 12 to 21 -> plane message to ask for departure

			sending:

				if index 4 is 0 -> ok

				if index 4 is 1 -> airport is closing

				if index 4 is 2 -> plane landed -> terminate

		if message type is 2-11 -> airport messages

			receiving:

				if index 4 is 1-> plane departed

				if index 4 is 2 -> plane landed

				if index 4 is -1 -> airport i closes

				if index 4 is 0 -> airport i opens

			sending:

				if all ok -> to departure airport

				if index 4 is 1 and all else is 0 -> asking to shut down airports

				if arrivals ==-1 and departure ==-1 : asking arrival airport to know that a plane is departing for the airport soon

				if index 4 is 1 -> pass to arrival airport to know that plane has departed 

	*/

	int running=1;

	while((running||running_cnt)){

		if (msgrcv(aid, (void *)&message, sizeof(message), 1,0) != -1){

			if(message.sender==1){

				running=0;

				printf("Closing msg received.\n");

				for(int i=1;i<11;i++){

					if(running_airports[i]){

						Message end_msg;

						end_msg.msg_type = i+1;

						end_msg.sender=1;

						end_msg.closes=1;

						end_msg.weight=0;

						if(msgsnd(aid,(void*)&end_msg,sizeof(Message),0)==-1){

							fprintf(stderr,"Error in sending closing Message\n");

						}

					}

				}

			}

			else if(message.sender>11){

				if(message.arrival_num==-1){

					running_cnt++;

					printf("plane process %d begins.\n",message.plane_id);

				}

				else if(message.arrival_num==-2){

					running_cnt--;

					printf("Plane process %d terminates.\n",message.plane_id);

				}

				else if(running ==0){

					Message m_to_p = message;

					

					m_to_p.sender=1;

					m_to_p.msg_type = message.plane_id+11;

					m_to_p.closes=1;

					if(msgsnd(aid, (void *)&m_to_p, sizeof(message), 0) == -1){

						fprintf(stderr, "Error in sending the plane details to ATC.\n");

					}

				}

				else{

					Message m_to_d = message;

					

					m_to_d.sender=1;

					m_to_d.msg_type = message.dept_num+1;

					if(msgsnd(aid, (void *)&m_to_d, sizeof(message), 0) == -1){

						fprintf(stderr, "Error in sending the plane details to ATC.\n");

					}

					

					Message m_to_a = message;

					

					m_to_a.sender=1;

					m_to_a.msg_type = message.arrival_num+1;

					m_to_a.arrival_num= -1;

					m_to_a.dept_num = -1;

					if(msgsnd(aid, (void *)&m_to_a, sizeof(message), 0) == -1){

						fprintf(stderr, "Error in sending the plane details to ATC.\n");

					}

					

					Message m_to_p = message;

					

					m_to_p.sender=1;

					m_to_p.msg_type = message.plane_id+11;

					m_to_p.closes=0;

					if(msgsnd(aid, (void *)&m_to_p, sizeof(message), 0) == -1){

						fprintf(stderr, "Error in sending the plane details to ATC.\n");

					}

				}

				

				

			}

			else{

				if(message.arrival_num==-1){

					running_cnt++;

					running_airports[message.plane_id]++;

					printf("Airport process %d begins.\n",message.plane_id);

				}

				else if(message.arrival_num==-2){

					running_cnt--;

					running_airports[message.plane_id]--;

					printf("Airport process %d terminates.\n",message.plane_id);

				}

				else if(message.closes==2){

					Message m_to_p = message;

					m_to_p.sender = 1;

					m_to_p.msg_type = m_to_p.plane_id+11;	

					if(msgsnd(aid, (void *)&m_to_p, sizeof(message), 0) == -1){

						fprintf(stderr, "Error in sending the plane details to ATC.\n");

					}

					Message m_to_a = message;

					m_to_a.sender=1;

					m_to_a.msg_type = message.arrival_num+1;

					m_to_a.arrival_num=-2;

					m_to_a.dept_num=-2;

					if(msgsnd(aid, (void *)&m_to_a, sizeof(message), 0) == -1){

						fprintf(stderr, "Error in sending the plane details to ATC.\n");

					}

				}

				else{

					Message m_to_a = message;

					m_to_a.sender = 1;

					m_to_a.msg_type = message.arrival_num+1;	

					m_to_a.closes = 1;

					if(msgsnd(aid, (void *)&m_to_a, sizeof(message), 0) == -1){

						fprintf(stderr, "Error in sending the plane details to ATC.\n");

					}

					Message m_to_d = message;

					m_to_d.sender=1;

					m_to_d.msg_type = message.dept_num+1;

					m_to_d.arrival_num=-2;

					m_to_d.dept_num=-2;

					if(msgsnd(aid, (void *)&m_to_d, sizeof(message), 0) == -1){

						fprintf(stderr, "Error in sending the plane details to ATC.\n");

					}	

					fprintf(file, "Plane %d has departed from Airport %d and will land at Airport %d.\n",m_to_a.plane_id,m_to_a.dept_num,m_to_a.arrival_num);

					fflush(file);

				}				

			}

		}

	}

	fclose(file);

	if (msgctl(aid, IPC_RMID, NULL) == -1) {

		fprintf(stderr,"Error in deleting message queue.\n");

		return 1;

	}

	else{

		printf("Message queue closed.\n");

	}

}
