#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "dyn_array.h"
#include "processing_scheduling.h"

#define FCFS "FCFS"
#define P "Priority"
#define RR "RR"
#define SRTF "SRTF"
#define SJF "SJF"

// FUNCTION DEFINITIONS

// Source Cited: https://www.geeksforgeeks.org/write-your-own-atoi/
// Implements atoi function
// \param str is a pointer to input string set to be sorted through
// \return the result from input string
int atoi_sifter(char *str);

// Prints visual statistics to readme.md and to the console
// \param fp if pointer to readme.md file
// \param sch_alg is a pointer to the specified scheduling algorithim input (FCFS, P, RR, SRTF, or SJF)
// \param result is a pointer to the ScheduleResult struct
void print_results(FILE *fp, const char *sch_alg, ScheduleResult_t *result);

int main(int argc, char **argv) 
{
    // verify enough was passed to proceed
    if (argc < 3 || argc > 4) 
    {
        printf("%s <pcb file> <schedule algorithm> [quantum]\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    // load process control blocks with 1st arg
    dyn_array_t *ready_queue = load_process_control_blocks(argv[1]);
    ScheduleResult_t *result = (ScheduleResult_t *)malloc(sizeof(ScheduleResult_t));

    //quantum and round robin set up even if not used cuz y not. Users are forgetful sometimes
    size_t quantum = 0;
    char rr[19] = "";
    if(argv[3]){// if quantum is provided
        quantum = atoi_sifter(argv[3]);
        snprintf(rr, 19, "RR: Quantum = %d", (int)quantum);
    } else// set quantum to 5 if not provided (default in process_scheduler)
        snprintf(rr, 19, "RR: Quantum = %d", 5);

    // set up statistics record to be added to readme file
    struct stat stats_record;
    FILE *fp = fopen("../readme.md", "a+"); // open as if appending
    stat("../readme.md", &stats_record);

    // set up statistics table header in readme.md
    //rough estimate of the initial text using https://charactercounttool.com/
    if(stats_record.st_size <= 600) {
        fprintf(fp, "\n\n| Scheduling Algorithm | Average Turnaround Time | Average Waiting Time | Total Clock Time |\n");
        fprintf(fp, "|----------------------|-------------------------|----------------------|------------------|\n");
    }
    //setup cases for each based on the "sch_alg provided in argv[]"
    // FCFS - First Come First Serve
    if(strcmp("FCFS", argv[2]) == 0 ){
        first_come_first_serve(ready_queue, result);
        print_results(fp, FCFS, result);
        printf("\nFCFS results written to readme.md!\n");
    }
    // P - Priority
    else if(strcmp("P", argv[2]) == 0 ){
        priority(ready_queue, result);
        print_results(fp, P, result);
        printf("\nPriority results written to readme.md!\n");
    }
    // RR - Round Robin
    else if(strcmp("RR", argv[2]) == 0 ){
        // could go without given quantum will always exist, but never hurts to be extra safe doesn't add any (significant)cost to runtime
        if(argc >= 0 && sscanf(argv[3], "%zu", &quantum)) {
            round_robin(ready_queue, result, quantum);
            print_results(fp, RR, result);
            printf("\nRR results written to readme.md!\n");
        } 
        else {
            fprintf(stderr, "ERROR: Invalid Quantum Input\n");
        }
    }
    // SRTF - Shortest Remaining Time First
    else if(strcmp("SRTF", argv[2]) == 0 ){
        shortest_remaining_time_first(ready_queue, result);
        print_results(fp, SRTF, result);
        printf("\nSRTF results written to readme.md!\n");
    }
    // SJF - Shortest Job First
    else if(strcmp("SJF", argv[2]) == 0 ){
        shortest_job_first(ready_queue, result);
        print_results(fp, SJF, result);
        printf("\nSJF results written to readme.md!\n");
    }
    //Default Case
    else{
        printf("%s <pcb file> <schedule algorithm> [quantum]\n", argv[0]);
    }
    
    // clean up, clean up, everybody do your share
    fclose(fp);
    dyn_array_destroy(ready_queue);
    free(result);

    return EXIT_SUCCESS;
}
//get quantum from input args[]
int atoi_sifter(char *str) {

    // Initialize result
    int res = 0;

    // Iterate through all characters 
    // of input string and update result
    for (int i = 0; str[i] != '\0'; ++i) 
        res = res * 10 + str[i] - '0'; 

    // return result
    return res;
}

//print out the result->times to the console and readme.md or wherever you want really based on the fp provided in main()
void print_results(FILE *fp, const char *sch_alg, ScheduleResult_t *result) {
    // print result to read me file
    fprintf(fp, "| %-20s | %-23f | %-20f | %-16lu |\n", sch_alg, result->average_turnaround_time, result->average_waiting_time, result->total_run_time);
    // print results for reference
    printf("\n| Scheduling Algorithm | Average Turnaround Time | Average Waiting Time | Total Clock Time |\n");
    printf("|----------------------|-------------------------|----------------------|------------------|\n");
    //pull variables from result of running sch_alg
    printf("| %-20s | %-23f | %-20f | %-16lu |\n", sch_alg, result->average_turnaround_time, result->average_waiting_time, result->total_run_time);
}