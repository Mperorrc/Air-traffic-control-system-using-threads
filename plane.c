#include <stdio.h>

#include <pthread.h>

#include <semaphore.h>

#include <math.h>

#include <stdlib.h> 

#include <unistd.h>

#include <string.h>

#include <sys/types.h>

#include <sys/wait.h>

#include <sys/ipc.h>

#include <sys/msg.h>



#define READ_END 0

#define WRITE_END 1

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

	key_t key;

	

	key = ftok("airtrafficcontroller.c", 'A');

	if (key == -1){

		fprintf(stderr,"error in creating atc message queue key\n");

		return 1;

	}

	

	int aid = msgget(key, 0644);    

	if (aid == -1){

		fprintf(stderr,"Error in creating atc message queue\n");

		return 1;

	}

	

	int plane_id;

	printf("Enter Plane ID:\n");

	scanf("%d",&plane_id);

	if(plane_id<0||plane_id>10){

		fprintf(stderr,"Invalid plane ID\n");

		return 1;

	}

	

	int plane_type;

	printf("Enter Type of Plane:\n");

	scanf("%d",&plane_type);

	if(plane_type<0||plane_type>1){

		fprintf(stderr,"Invalid plane type\n");

		return 1;

	}

	

	int total_weights=0;

	

	if(plane_type){

		int num_passengers;

		printf("Enter Number of Occupied Seats:\n");

		scanf("%d",&num_passengers);

		if(num_passengers<1||num_passengers>10){

			fprintf(stderr,"Invalid passenger number\n");

			return 1;

		}

		

		int passenger_to_plane[num_passengers][2];

		int fork_val = 1;

		int cid=0;

		

		for(int i=0;i<num_passengers;i++){

			if( pipe(passenger_to_plane[i]) == -1){

				fprintf(stderr,"Pipe creation failed\n");

				return 1;



			}

		}



		

		for(int i=0;i<num_passengers&&fork_val;i++){

			cid=i;

			fork_val = fork();

			if(fork_val < 0){

				fprintf(stderr,"Fork Failed\n");

				return 1;

			}

		}

		

		if(fork_val){

			int luggage_weights = 0;

			int passenger_weights = 0;

			for(int i=0;i<num_passengers;i++){

				int getting_weights = 1;

				int read_weights[10]={0};

				close(passenger_to_plane[i][WRITE_END]);

				while(getting_weights){

					read(passenger_to_plane[i][READ_END],read_weights,10);

					if(read_weights[1]!=0){

						getting_weights = 0;

						luggage_weights += read_weights[0];

						passenger_weights += read_weights[1];

					}

				}

				close(passenger_to_plane[i][READ_END]);

			}

			

			printf("Luggage weights : %d kg, passenger body weights : %d kg\n",luggage_weights,passenger_weights);

			total_weights = luggage_weights+passenger_weights + (75*7);

			for(int i=0;i<num_passengers;i++){

				wait(NULL);

			}

		}

		else{

			sleep(cid*15);

			int luggage_weight;

			printf("Enter Weight of Your Luggage:\n");

			scanf("%d",&luggage_weight);

			if(luggage_weight<0||luggage_weight>25){

				fprintf(stderr,"Invalid weight\n");

				return 1;

			}

			

			int person_weight;

			printf("Enter Your Body Weight:\n");

			scanf("%d",&person_weight);

			if(person_weight<10||person_weight>100){

				fprintf(stderr,"Invalid weight\n");

				return 1;

			}

			int write_wts[10]={0};

			write_wts[0]=luggage_weight;

			write_wts[1] = person_weight;

			close(passenger_to_plane[cid][READ_END]);

			write(passenger_to_plane[cid][WRITE_END],write_wts,10);

			close(passenger_to_plane[cid][WRITE_END]);

			return 0;

			

		}

	}

	else{

		int cargo_cnt;

		printf("Enter Number of Cargo Items:\n");

		scanf("%d",&cargo_cnt);

		if(cargo_cnt<0||cargo_cnt>100){

			fprintf(stderr,"Invalid number of cargo items.\n");

			return 1;

		}

		int cargo_wt;

		printf("Enter Average Weight of Cargo Items:\n");

		scanf("%d",&cargo_wt);

		if(cargo_cnt<0||cargo_wt>100){

			fprintf(stderr,"Invalid cargo weights.\n");

			return 1;

		}

		total_weights = (75*2) + (cargo_cnt*cargo_wt); 

	}

	printf("Total plane weight : %d kg\n", total_weights);

	

	int departure_airport_num;

	printf("Enter Airport Number for Departure:\n");

	scanf("%d",&departure_airport_num);

	if(departure_airport_num<=0||departure_airport_num>10){

		fprintf(stderr,"Invalid airport number.\n");

		return 1;

	}

	

	int arrival_airport_num;

	printf("Enter Airport Number for Arrival:\n");

	scanf("%d",&arrival_airport_num);

	if(arrival_airport_num<=0||arrival_airport_num>10){

		fprintf(stderr,"Invalid airport number.\n");

		return 1;

	}

	

	Message message;

	

	message.msg_type = 1;

	message.plane_id = plane_id;

	message.arrival_num = arrival_airport_num;

	message.dept_num = departure_airport_num;

	message.weight = total_weights;

	message.closes = 0;

	message.sender = 11+plane_id;

	printf("Total Weight : %d\n",message.weight);

	if(msgsnd(aid, (void *)&message, sizeof(Message), 0) == -1){

		fprintf(stderr, "Error in sending the plane details to ATC.\n");

		return 1;

	}

	Message flight_details;

	if(msgrcv(aid, (void *)&flight_details, sizeof(Message), plane_id+11,0) == -1){

		fprintf(stderr, "Error in sending the plane details to ATC.\n");

		return 1;

	}

	if(flight_details.closes==1){

		printf("Airports closing.\n");

		return 0;

	}

	

	Message start_msg;

	start_msg.arrival_num=-1;

	start_msg.plane_id = plane_id;

	start_msg.msg_type=1;

	start_msg.sender = plane_id+11;

	if(msgsnd(aid, (void *)&start_msg, sizeof(start_msg), 0) == -1){

		fprintf(stderr, "Error in sending the plane details to ATC.\n");

	}

	

	Message landing_status;

	if (msgrcv(aid, (void *)&landing_status, sizeof(Message), plane_id + 11,0) == -1){

		fprintf(stderr,"Error in receiving the departure confirmation.\n");

		return 1;

	}

	printf("Plane %d has successfully traveled from Airport %d to Airport %d!\n",plane_id,departure_airport_num,arrival_airport_num);

	Message end_msg;

	end_msg.arrival_num=-2;

	end_msg.plane_id = plane_id;

	end_msg.msg_type=1;

	end_msg.sender = plane_id+11;

	if(msgsnd(aid, (void *)&end_msg, sizeof(Message), 0) == -1){

		fprintf(stderr, "Error in sending the plane details to ATC.\n");

	}	

}
