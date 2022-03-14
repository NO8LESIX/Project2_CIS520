#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dyn_array.h"
#include "processing_scheduling.h"

#define QUANTUM 5

// private function
void virtual_cpu(ProcessControlBlock_t *process_control_block) 
{
    // decrement the burst time of the pcb
    --process_control_block->remaining_burst_time;
}
//calculates remaining burst time
int burst_time_calculator(const void *pcb1, const void *pcb2) {
    return ((ProcessControlBlock_t *)pcb1)->remaining_burst_time - ((ProcessControlBlock_t *)pcb2)->remaining_burst_time;
}
//Iterates over all processes in queue 
void pcb_queuer(dyn_array_t *ready_queue, dyn_array_t *dyn_arr_queue, uint32_t run_time) {
    size_t size = dyn_array_size(ready_queue);
    for(size_t i = 0; i < size; i++) {
        ProcessControlBlock_t pcb;
        dyn_array_extract_back(ready_queue, &pcb);
        if(pcb.arrival <= run_time)
            dyn_array_push_back(dyn_arr_queue, &pcb);
        else
            dyn_array_push_front(ready_queue, &pcb);
    }
}
//sorts queue by remaining burst time
void pcb_queue_sort_by_time(dyn_array_t *ready_queue, dyn_array_t *dyn_arr_queue, uint32_t run_time) {
    pcb_queuer(ready_queue, dyn_arr_queue, run_time);
    dyn_array_sort(dyn_arr_queue, burst_time_calculator);
}
//compare two pcb's burst time and return diference
int pcb_compare_burst_time(const void *pcb1, const void *pcb2) {
    
    uint32_t a = ((ProcessControlBlock_t *)pcb1)->arrival;
    uint32_t b = ((ProcessControlBlock_t *)pcb2)->arrival;
    if(a == b)
        return ((ProcessControlBlock_t *)pcb1)->remaining_burst_time - ((ProcessControlBlock_t *)pcb2)->remaining_burst_time;
    return b - a;
}
//compares the priorities of two pcbs, return the smaller
int compare_priorities(const void* pcb1, const void* pcb2){
    return((ProcessControlBlock_t*)pcb1)->priority > ((ProcessControlBlock_t*)pcb2)->priority ? -1
        : ((ProcessControlBlock_t*)pcb1)->priority < ((ProcessControlBlock_t*)pcb2)->priority ? 1
        :  0;
}
//compare remaining_burst_time of two pcbs and get the lesser
int get_shorter_burst_time(const void *pcb1, const void *pcb2)
{
    
    if (((ProcessControlBlock_t *)pcb1)->remaining_burst_time > ((ProcessControlBlock_t *)pcb2)->remaining_burst_time)
        return -1;
    else if (((ProcessControlBlock_t *)pcb1)->remaining_burst_time < ((ProcessControlBlock_t *)pcb2)->remaining_burst_time)
        return 1;
    return 0;
}

bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
   //quick catch
   if(!ready_queue || !result)
        return false;
    //declare all vars as uInt32s for ease later, ready_queue is never NUll or empty thanks to catch, set everything else to 0
    uint32_t size = dyn_array_size(ready_queue);
    uint32_t waiting_time = 0;
    uint32_t turnaround_time = 0;
    uint32_t run_time = 0;

    ProcessControlBlock_t pcb;
    
    //run based on outline in .h
    for(uint32_t i = 0; i < size; i++) {
        waiting_time += run_time;
        dyn_array_extract_back(ready_queue, (void *)&pcb);
        run_time += pcb.remaining_burst_time;
        turnaround_time += run_time;
        turnaround_time -= pcb.arrival;
        waiting_time -= pcb.arrival;

        while(pcb.remaining_burst_time > 0)
            virtual_cpu(&pcb);
    }
    //assign local results to the struct
    result->average_waiting_time = (float)waiting_time / (float)size;
    result->average_turnaround_time = (float)turnaround_time / (float)size;
    result->total_run_time = run_time;
    //exit gracefully
    return true;
}

bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    //same great catch, all new look
    if(ready_queue == false || result == false) return false;
        ProcessControlBlock_t pcb;
        //sort based on priorites before running core
        int (*const compare_priority)(const void*, const void*) = &compare_priorities;
        dyn_array_sort(ready_queue, compare_priority);
        //declare all vars as uInt32s for ease later, ready_queue is never NUll or empty thanks to catch, set everything else to 0        
        uint32_t size = dyn_array_size(ready_queue);
        uint32_t waiting_time = 0;
        uint32_t turnaround_time = 0;
        uint32_t run_time = 0;
        //run core based on the .h description
        for(uint32_t i = 0; i < size; i++){
          run_time += waiting_time;
          dyn_array_extract_back(ready_queue, (void*)&pcb);
          waiting_time += pcb.remaining_burst_time;
          turnaround_time += waiting_time;
          while(!pcb.remaining_burst_time){
            virtual_cpu(&pcb);
          }
        }
        //store local results into result
        result->average_waiting_time = (float)waiting_time / size;
        result->average_turnaround_time = (float)turnaround_time / size;
        result->total_run_time = run_time;
        //exit gracefully
        return true;
}

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
    //verify input, quantum must be a positive int
    if(!ready_queue || !result || quantum <= 0)
        return false;
    //declare all vars as uInt32s for ease later, ready_queue is never NUll or empty thanks to catch, set everything else to 0
    uint32_t size = dyn_array_size(ready_queue);
    dyn_array_t *dyn_arr_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    uint32_t waiting_time = 0;
    uint32_t turnaround_time = 0;
    uint32_t run_time = 0;
    uint32_t current_run_time = 0;
    
    //a little bit of priority assignment for the aroma
    for (uint32_t i = 0; i < size; i++) {
        ProcessControlBlock_t *pcb_ptr = (ProcessControlBlock_t *)dyn_array_at(ready_queue, i);
        pcb_ptr->priority = pcb_ptr->arrival;
    }
    //queue up the ingrediente
    pcb_queuer(ready_queue, dyn_arr_queue, run_time);
    //execute based on the good 'ol .h cookbook
    while(dyn_array_size(dyn_arr_queue) != 0) {
        ProcessControlBlock_t pcb;
        dyn_array_extract_front(dyn_arr_queue, &pcb);
        waiting_time += run_time - pcb.arrival;

        if(pcb.remaining_burst_time > quantum)
            current_run_time = quantum;
        else
            current_run_time = pcb.remaining_burst_time;

        run_time += current_run_time;

        while(current_run_time > 0) {
            virtual_cpu(&pcb);
            current_run_time--;
        }

        pcb.arrival = run_time;

        pcb_queuer(ready_queue, dyn_arr_queue, run_time);

        if(pcb.remaining_burst_time == 0)
            turnaround_time += run_time - pcb.priority;
        else
            dyn_array_push_back(dyn_arr_queue, &pcb);
    }
    //assign vars to the usual
    result->average_waiting_time = (float)waiting_time / size;
    result->average_turnaround_time = (float)turnaround_time / size;
    result->total_run_time = run_time;
    //little bit of cleaning up under the rug and exit gracefully
    dyn_array_destroy(dyn_arr_queue);
    return true;
}


dyn_array_t *load_process_control_blocks(const char *input_file) 
{
    //return Null if file is Null
    if(input_file == NULL) return NULL;
    
    //little more Null checking, a a treat
    char* bypass;
    bypass = strrchr(input_file, '.');
    if(bypass == NULL || strcmp(bypass, ".bin")) return NULL;
    int file_descriptor = open(input_file, O_RDONLY);
    if(file_descriptor <= 0){
      close(file_descriptor);
      return NULL;
    }
    
    uint32_t length;
    int size = read(file_descriptor, &length, sizeof(uint32_t));
    if(size <= 0) return NULL;
    
    ProcessControlBlock_t* pcb = malloc(sizeof(ProcessControlBlock_t) * length);
    
    //litle file reader action here
    for(uint32_t i = 0; i < length; i++){
      size = read(file_descriptor, &pcb[i].remaining_burst_time, sizeof(uint32_t));
      size = read(file_descriptor, &pcb[i].priority, sizeof(uint32_t));      
    }
    
    dyn_array_t* dynamic_array = dyn_array_import((void*) pcb, sizeof(length), sizeof(ProcessControlBlock_t), NULL);
    //windows shutdown noises
    close(file_descriptor);
    free(pcb);
    //return the new loaded array
    return dynamic_array;
}

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    //the basic 1-2 catch
    if(!ready_queue || !result)
        return false;

    uint32_t size = dyn_array_size(ready_queue);
    //set priority ready_queue
    for(uint32_t i = 0; i < size; i++) {
        ProcessControlBlock_t *pcb_ptr = (ProcessControlBlock_t *)dyn_array_at(ready_queue, i);
        pcb_ptr->priority = pcb_ptr->arrival;
    }
    //load vars
    dyn_array_t *dyn_arr_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    uint32_t waiting_time = 0;
    uint32_t turnaround_time = 0;
    uint32_t run_time = 0;
    //housekeeping with some sorts
    dyn_array_sort(ready_queue, pcb_compare_burst_time);
    pcb_queue_sort_by_time(ready_queue, dyn_arr_queue, run_time);
    //main course described in the .h
    while(dyn_array_size(dyn_arr_queue) != 0) {
        ProcessControlBlock_t pcb;
        dyn_array_extract_front(dyn_arr_queue, &pcb);

        waiting_time += run_time - pcb.arrival;
        uint32_t ready_queue_size = dyn_array_size(ready_queue);
        uint32_t current_run_time;

        if(ready_queue_size != 0) {
            uint32_t arrival_n;
            arrival_n = ((ProcessControlBlock_t *)dyn_array_at(ready_queue, dyn_array_size(ready_queue) - 1))->arrival;
            current_run_time = arrival_n - run_time;
        } else
            current_run_time = pcb.remaining_burst_time;

        run_time += current_run_time;

        while(current_run_time > 0) {
            virtual_cpu(&pcb);
            current_run_time--;
        }
        pcb.arrival = run_time;

        if(pcb.remaining_burst_time == 0)
            turnaround_time += run_time - pcb.priority;
        else
            dyn_array_push_back(dyn_arr_queue, &pcb);

        pcb_queue_sort_by_time(ready_queue, dyn_arr_queue, run_time);
    }
    //set results 
    result->average_waiting_time = (float)waiting_time / size;
    result->average_turnaround_time = (float)turnaround_time / size;
    result->total_run_time = run_time;
    //clean house and give the green light
    dyn_array_destroy(dyn_arr_queue);
    return true;
}


bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    //same look, same classic taste of error checking
    if(!ready_queue || !result)
        return false;
    //ensure that something is returned, hiccups averted cuz something is printed
    result->average_waiting_time = 0;
    result->average_turnaround_time = 0;
    result->total_run_time = 0;

    //initialize vars baybeeee
    bool extract = false;
    uint32_t size = dyn_array_size(ready_queue);
    uint32_t clock_time = 0;
    uint32_t total_latency = 0;
    uint32_t turnaround_time = 0;
    int arrival_sum = 0;

    ProcessControlBlock_t *pcb_ptr = malloc(sizeof(ProcessControlBlock_t));
    pcb_ptr = (ProcessControlBlock_t *)dyn_array_export(ready_queue);
    dyn_array_t *dyn_arr_queue = dyn_array_create(0, size, NULL);

    //little bit of snacking based on the .h file
    for(uint32_t i = 0; i < size; i++) {
        for(uint32_t j = 0; j < size; j++) {

            if(pcb_ptr[j].arrival <= clock_time && pcb_ptr[j].started == false) {

                extract = dyn_array_push_back(dyn_arr_queue, &pcb_ptr[j]);
                if(extract == false)
                    return false;

                arrival_sum += pcb_ptr[j].arrival;
                pcb_ptr[j].started = true;
            }
        }

        if(dyn_array_size(dyn_arr_queue) > 0) {
            
            dyn_array_sort(dyn_arr_queue, get_shorter_burst_time);

            while(dyn_array_size(dyn_arr_queue) > 0) {
                
                ProcessControlBlock_t pcb;
                extract = dyn_array_extract_back(dyn_arr_queue, &pcb);
                if(extract == false)
                    return false;

                
                total_latency += clock_time;
                pcb.started = true;

               
                while(pcb.remaining_burst_time != 0) {
                    virtual_cpu(&pcb);
                    clock_time++;       
                }

                pcb.started = false;
                turnaround_time += clock_time;
            }
        }
    }
    //Print the check and close for the night, you don't have to go home but you can't stay here babe
    result->average_waiting_time = (float)(total_latency - arrival_sum) / size;
    result->average_turnaround_time = (float)(turnaround_time - arrival_sum) / size;
    result->total_run_time = clock_time;
    return true;
}
