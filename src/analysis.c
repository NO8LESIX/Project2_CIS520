#include <stdio.h>
#include <stdlib.h>

#include "dyn_array.h"
#include "processing_scheduling.h"

#define FCFS "FCFS"
#define P "P"
#define RR "RR"
#define SJF "SJF"

int main(int argc, char **argv) 
{
    if (argc < 3) 
    {
        printf("%s <pcb file> <schedule algorithm> [quantum]\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    dyn_array_t* dyn_array = load_process_control_blocks(argv[1]);
    
    ScheduleResult_t* = (ScheduleResult_t) malloc(sizeof(ScheduleResult_t));
    
    result->average_latency_time = 0;
    result->average_wall_clock_time = 0;
    result->total_run_time = 0;
    
    if(strcmp("FCFS", argv[2]) == 0){
      printf("FCFS executing...\n");
      first_come_first_serve(dyn_array, result);
    }
    
    if(strcmp("P", argv[2]) == 0){
      printf("Priority executing...\n");
      priority(dyn_array, result);
    }
    
    printf("Average Letency: %f\n", result->average_latency_time);
    printf("Average Wall Clock Time: %f\n", result->average_wall_clock_time);
    printf("Total Runtime: %f\n", result->total_run_time);
    
    //trash collection
    dyn_array_destroy(dyn_array);
    free(result);

    return EXIT_SUCCESS;
}
