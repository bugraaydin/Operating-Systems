#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#define READ 0
#define WRITE 1
////************************************************************************************//////
////https://www.geeksforgeeks.org/queue-set-2-linked-list-implementation//////
///*************************************************************************************/////
struct QNode
{
    int key;
    struct QNode *next;
};
 
// The queue, front stores the front node of LL and rear stores ths
// last node of LL
struct Queue
{
    struct QNode *front, *rear;
};
 
// A utility function to create a new linked list node.
struct QNode* newNode(int k)
{
    struct QNode *temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->key = k;
    temp->next = NULL;
    return temp; 
}
 
// A utility function to create an empty queue
struct Queue *createQueue()
{
    struct Queue *q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}
 
// The function to add a key k to q
void enqueue(struct Queue *q, int k)
{
    // Create a new LL node
    struct QNode *temp = newNode(k);
 
    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL)
    {
       q->front = q->rear = temp;
       return;
    }
 
    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}
 
// Function to remove a key from given queue q
int dequeue(struct Queue *q)
{
    // If queue is empty, return NULL.
    if (q->front == NULL)
       return -1;
 
    // Store previous front and move front one node ahead
    struct QNode *temp = q->front;
    q->front = q->front->next;
 
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
       q->rear = NULL;
    return temp->key;
}

////************************************************************************************//////
////END OF QUEUE IMPLEMENTATION, STARTING THE ASSIGNMENT CODE/////
///*************************************************************************************/////



int main(int argc, char** argv){
	//ATOI FUNCTION CONVERTING LINUX COMMENT INTO DESIRED ATTRIBUTES
	int maxInteger = atoi(argv[1]);
	int childCount = atoi(argv[2]);
	//i for loops
	int i = 0;
	//printf("Max integer = %d ||||| Child Count = %d\n",maxInteger,childCount);
	////////////////////////////////////////////////////////////////
	if((maxInteger <= 1000000 && maxInteger >= 10) &&
						(childCount > 0 && childCount < 51)){
		//creating childCount + 2 pipes ( +2 for mp and pr)
		int fd[childCount+2][2];
		for(i = 0;i < (childCount+2);i++){ 
			pipe(fd[i]); //?
		}
		fcntl(fd[childCount+1][READ], F_SETFL, O_NONBLOCK);
		fcntl(fd[childCount+1][WRITE], F_SETFL, O_NONBLOCK);
		
		//creating childCount + 1 child process (+1 for pr)
		pid_t processes[childCount+1]; //index of pr is childcount
		pid_t thisOne;
		int thisIndex;
		int prime;
		for(i = 0; i < childCount+1;i++){
			processes[i] = fork();
			thisOne = processes[i];
			thisIndex = i;
			prime = 0;
			if(processes[i] == 0){
				if(thisIndex == childCount){
					//pr
					//close(fd[i][WRITE]); //write port closed
					break;
				}
				//childs
				//close(fd[i][READ]);
				break; //exit loop
			}
			else if(processes[i] > 0){
				//parent
				//close(fd[i][READ]);
			}
			else 
				fprintf(stderr,"Invalid fork");
		}
		//parent MP
		if(thisOne > 0){
			int data;
			int flag = -2;
			int terminateFlag = -1;
			//printf("I'm a fucking parent\n");
			//creating queue
			struct Queue *head = createQueue();
			for(i = 2; i <= maxInteger;i++){
				enqueue(head,i);
			}
			enqueue(head,flag);
			//dequeue'ing first integer
			while(1){
				data = dequeue(head);
				if((data > 1) ||(data == -2)){
					//printf("I am parent, I am sending %d to C1\n",data);
					write(fd[0][WRITE],&data,sizeof(data));
					//continue;
				}
				int toPr;
				if(read(fd[childCount+1][READ],&toPr,sizeof(toPr)) == sizeof(toPr)){
					//printf("Main: %d\n",toPr);
					if(toPr == -3){
						//printf("brrra\n");
						int status;
						toPr = -3;
						write(fd[0][WRITE],&toPr,sizeof(toPr));
					}
					if(toPr == -5)
						break;
					//printf("I'm parent, I'm reading %d\n",toPr);
					enqueue(head,toPr);
				}
			}
			////////////////*****************//////////////////////
			//waiting for other processes to complete before terminating
			/*for(i = 0; i < childCount+1;i++){
				wait(0);
			}*/
		}
		else if(thisIndex == childCount){ //PR PROCESS
			int toPrint;
			while(1){
				if((read(fd[childCount][READ],&toPrint,sizeof(toPrint))) == sizeof(toPrint)){
					if(toPrint == -1){
						//printf("-1 okudum prı kapıyom\n");
						break;
					}
					printf("%d\n",toPrint);
					fflush(stdout);
					}
			}
		}
		//child
		else{
			int toWrite;
			while(1){
				if((read(fd[thisIndex][READ],&toWrite,sizeof(toWrite))) == sizeof(toWrite)){
					///////
					//printf("My prime is : %d\n",prime);
					///
					if(toWrite == -3){ ///KILLING CHILDS SIGNAL RECEIVED, SENDING -3 TO ALL CHILDS AND -5 TO MP
						if(thisIndex == childCount - 1){ //cm 
							toWrite = -5; //SENDING -5 TO MP
							write(fd[childCount+1][WRITE],&toWrite,sizeof(toWrite));
						}
						else{ //other childs
							write(fd[thisIndex+1][WRITE],&toWrite,sizeof(toWrite));	
						}
						fflush(stdout);
						//printf("çocuk halimle açlıktan öldüm :(\n");
						fflush(stdout);
						break;
					}
					if((toWrite == -2) && (prime == 0)){
						if(thisIndex == childCount - 1){ //cm 
							//printf("im cm sending terminal signal -3\n");
							toWrite = -3; //SENDING TERMINATING SIGNAL TO ALL MAIN
							write(fd[childCount+1][WRITE],&toWrite,sizeof(toWrite));
						}
						else{ //other childs
							toWrite = -2;
							write(fd[thisIndex+1][WRITE],&toWrite,sizeof(toWrite));	
						}
						toWrite = -1;
						write(fd[childCount][WRITE],&toWrite,sizeof(toWrite)); //writing terminate to pr
						continue;
					}
					if((toWrite == -2) && (prime != 0)){
						//printf("im reading -2\n");
						prime = 0;
						if(thisIndex == childCount - 1){ //cm 
							write(fd[childCount+1][WRITE],&toWrite,sizeof(toWrite));
						}
						else{ //other childs
							write(fd[thisIndex+1][WRITE],&toWrite,sizeof(toWrite));	
						}
						continue;
					}
					///////
					if(prime == 0){ // first case where prime is received
						prime = toWrite;
					}
					if((toWrite % prime == 0) && (prime != toWrite) && toWrite != -1){ //if the read value is not prime keep reading
						//printf("test\n");
						//read(fd[thisIndex][READ],&toWrite,sizeof(toWrite));
						continue;
					}
					else if(toWrite % prime != 0){ 
						//printf("kekoslavakya\n");
						if(thisIndex == childCount - 1){ //cm 
							//printf("Writing %d\n",toWrite);
							write(fd[childCount+1][WRITE],&toWrite,sizeof(toWrite));
						}
						else{ //other childs
							write(fd[thisIndex+1][WRITE],&toWrite,sizeof(toWrite));	
						}	
					}
					else{ //if a prime number is found send it to pr
							write(fd[childCount][WRITE],&toWrite,sizeof(toWrite)); //writing found prime to pr	
					}
				}
			}
		}
	}
	else{
		printf("\n Invalid inputs");
	}
	return 0;
}
