#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "disk_request.h"

#define max(x, y) ((x) > (y)? (x) : (y))
#define INF INT_MAX
#define MAX_REQUESTS 500

enum direction{RIGHT = 1, LEFT = 0}; 

// reads the input
int read_input(disk_request_t* disk_requests, char* filename) {
    FILE *input_file = fopen(filename, "r");
    int i = 0; 
    disk_request_t *current = &disk_requests[i];
    while (fscanf(input_file, "%d %d", &current->time, &current->cylinder) != EOF) {
		i++;
		current = &disk_requests[i];
	}
	return i;
}

void print_statistics(char* function_name, int total_time, int *wait_times, int n) { 
    double avg = 0, sd = 0;
    int i;
	printf("\n");
    for (i = 0; i < n; ++i) {
        //printf("Request Index: %d, Wait Tİme%d\n", i,wait_times[i]);
        avg += wait_times[i]/(double)n;
    }
    for (i = 0; i < n; ++i)
        sd += (wait_times[i] - avg) * (wait_times[i] - avg);
    
    sd = sqrt(sd/(n-1));

    printf("%s: %d	%.6lf	%.6lf",function_name, total_time, avg, sd);
}

// compare function used for quick sort
int compare(const void *elem1, const void *elem2) {
    disk_request_t *first_element = (disk_request_t *) elem1;
    disk_request_t *second_element = (disk_request_t *) elem2;
    if (first_element->time == second_element->time)
        return first_element->cylinder > second_element->cylinder; 
    return first_element->time > second_element->time; 
}

// simply prints out a given disk_request_t array
void print_disk_requests(disk_request_t *disk_requests, int n) {
    int i;
    for (i = 0; i < n; ++i)
        printf("%d. time: %d, cylinder: %d\n", i+1, disk_requests[i].time, disk_requests[i].cylinder);
}

void simulate_fcfs(disk_request_t *disk_requests, int n) {
    int current_time = 0; 
    int current_cylinder = 1;
    int i, wait_times[n];
    
     
    for (i = 0; i < n; ++i) {
        wait_times[i] = max(0, current_time - disk_requests[i].time);
           //+ (int)abs(current_cylinder - disk_requests[i].cylinder);
        current_time = max(current_time, disk_requests[i].time);
	current_time += (int)abs(current_cylinder - disk_requests[i].cylinder);
        current_cylinder = disk_requests[i].cylinder;
    }

    print_statistics("FCFS", current_time, wait_times, n);
}  

void simulate_look(disk_request_t *disk_requests, int n) {
    int current_time = 0;
    int current_cylinder = 1;
    int i, direction = RIGHT;
    int visited[n], wait_times[n];

    for (i = 0; i < n; ++i) 
        visited[i] = 0;

	while(1) {
		int closest_dist = INF;
		int closest_index = -1;
         
		if (direction == RIGHT) {
			for (i = 0 ; i < n; ++i)
                if (!visited[i] && disk_requests[i].time <= current_time && disk_requests[i].cylinder > current_cylinder) 
					if (disk_requests[i].cylinder - current_cylinder < closest_dist) {		
						closest_dist = disk_requests[i].cylinder - current_cylinder;
						closest_index = i;
					}
		}
        else {
			for (i = 0 ; i < n; ++i)
		        if (!visited[i] && disk_requests[i].time <= current_time && disk_requests[i].cylinder < current_cylinder) 
					if (current_cylinder - disk_requests[i].cylinder < closest_dist) {
		            	closest_dist = current_cylinder - disk_requests[i].cylinder;
						closest_index = i; 
					}
		}
		if (closest_index == -1) {
			for (i = 0; i < n; ++i)
				if (!visited[i] && current_time >= disk_requests[i].time) {
					direction = 1 - direction;
					break;
				}
			// if the queue is empty 
			if (i == n) {
				for (i = 0; i < n; ++i)
					if (!visited[i]) {
						break;
					}
				if (i == n)
					break;
				current_time = disk_requests[i].time;
				if (disk_requests[i].cylinder > current_cylinder)
					direction = RIGHT;
				else 
					direction = LEFT;
			}
		}
		// process this entry 
		else {
			wait_times[closest_index] = current_time - disk_requests[closest_index].time;
			visited[closest_index] = 1;
			//printf("%d~~~",closest_index);
			current_time += closest_dist;
			current_cylinder = disk_requests[closest_index].cylinder;
		}
	}
	

    print_statistics("LOOK", current_time, wait_times, n);
}

void simulate_sstf(disk_request_t *disk_requests, int n) { 
    int current_time = 0;
    int current_cylinder = 1;

    int i, wait_times[n];
    int visited[n], count = 0;

    for (i = 0; i < n; ++i)
        visited[i] = 0;
 
    while (count != n) {
        int closest_index = -1, closest_distance = INF;

        // find the closest disk request that is in the queue
        for (i = 0; i < n; ++i) 
            if (!visited[i] && (int)abs(disk_requests[i].cylinder - current_cylinder) < closest_distance
                && disk_requests[i].time <= current_time) {
                    closest_index = i;
                    closest_distance = (int)abs(disk_requests[i].cylinder - current_cylinder);
                }
        
        // if there's nothing in the queue, wait for a new request to arrive
        if (closest_index == -1) {
            for (i = 0; i < n; ++i) 
                if (!visited[i]) {
                    closest_index = i;
                    closest_distance = (int)abs(disk_requests[i].cylinder - current_cylinder);
                    break;
                }
        }

        current_cylinder = disk_requests[closest_index].cylinder;
        visited[closest_index] = 1; 
        wait_times[closest_index] = max(0, current_time - disk_requests[closest_index].time); //+ closest_distance;
		current_time = max(disk_requests[i].time,current_time);
        current_time += closest_distance;
        count++;
    }

    print_statistics("SSTF", current_time, wait_times, n);
}

void simulate_clook(disk_request_t *disk_requests, int n) {
    int current_time = 0;
    int current_cylinder = 1;
    int i;
    int visited[n], wait_times[n];

    for (i = 0; i < n; ++i) 
        visited[i] = 0;

	while(1) {
		int closest_dist = INF;
		int closest_index = -1;
		for (i = 0 ; i < n; ++i)
            if (!visited[i] && disk_requests[i].time <= current_time && disk_requests[i].cylinder >= current_cylinder) 
				if (disk_requests[i].cylinder - current_cylinder < closest_dist) {		
					closest_dist = disk_requests[i].cylinder - current_cylinder;
					closest_index = i;
				}
		if (closest_index == -1) {
			for(i = 0; i < n; i++){
				if(visited[i] == 0 && disk_requests[i].time <= current_time)
					break;
			}
			if(i == n){
				for(i = 0; i < n; i++){
					if(visited[i] == 0){
						current_time = disk_requests[i].time;
						break;
					}
				}
				if(i == n)
					break;
			}
			else{
				int last_cylinder = current_cylinder;
				int last_index;
				for(i = 0; i < n; i++){
					if(visited[i] == 0 && disk_requests[i].time <= current_time){
						if(disk_requests[i].cylinder <= last_cylinder){
							//current_time = current_cylinder - disk_requests[i].cylinder;
							last_cylinder = disk_requests[i].cylinder;
							last_index = i;
						}
					}
				}
				wait_times[last_index] = current_time - disk_requests[last_index].time;
				visited[last_index] = 1;
				current_time += current_cylinder - last_cylinder;
				current_cylinder = last_cylinder;
				
			}
		}
		// process this entry 
		else {	
			wait_times[closest_index] = current_time - disk_requests[closest_index].time;
			visited[closest_index] = 1;
			current_time += closest_dist;
			current_cylinder = disk_requests[closest_index].cylinder;
		}
	}
    print_statistics("C-LOOK", current_time, wait_times, n);
}



int main(int argc, char **argv) {
    char *filename = argv[2];
    disk_request_t *disk_requests = malloc(sizeof(disk_request_t)*MAX_REQUESTS); 
    
    // read the input
    int n = read_input(disk_requests, filename);
    
    // sort disk_requests array in terms of time and cylinder
    // smaller arrival time comes first 
    // if arrival times are same smaller cylinder comes first


    //print_disk_requests(disk_requests, n);

    simulate_fcfs(disk_requests, n);
    simulate_sstf(disk_requests, n);
    simulate_look(disk_requests, n);
    simulate_clook(disk_requests, n);
    printf("\n\n");
    free(disk_requests);
    return 0;
}
