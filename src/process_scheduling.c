#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dyn_array.h"
#include "processing_scheduling.h"


// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)
#define QUANTUM 5

// private function
void virtual_cpu(ProcessControlBlock_t *process_control_block) 
{
    // decrement the burst time of the pcb
    --process_control_block->remaining_burst_time;
}

bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    //nothing to process
    if(ready_queue == NULL || result == NULL) return false;
    
    //initialize vars
    ProcessControlBlock_t pcb;
    
    int burst_time = 0;
    int wall_time = 0;
    int latency = 0;
    int capacity = (int) dyn_array_size(ready_queue);
    //end vars
    
    //process ready queue
    for(int i = 0; i < capacity; i++){
      latency += burst_time;
      dyn_array_extract_back(ready_queue, (void*)&pcb);
      burst_time += pcb.remaining_burst_time;
      wall_time += burst_time;
      while(!pcb.remaining_burst_time){
        virtual_cpu(&pcb);
      }
    }
    
    result->average_latency_time = (float)latency/capacity;
    result->average_wall_clock_time = (float)wall_time/capcity;
    result->total_run_time = (float)burst_time;
    
    return true;
}

bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if(ready_queue == false || result == false) return false;

    ProcessControlBlock_t pcb;
        
    //compare two priorities of PCBs in method
    
    int capacity = (int) dyn_array_size(ready_queue);
    int burst_time = 0;
    int wall_time = 0;
    int latency = 0;    
    
    for(int i = 0; i < capacity; i++){
      latency += burst_time;
      dyn_array_extract_back(ready_queue, (void*)&pcb);
      burst_time += pcb.remaining_burst_time;
      wall_time += burst_time;
      while(!pcb.remaining_burst_time){
        virtual_cpu(&pcb);
      }
    }
    
    result->average_latency_time = (float)latency/capacity;
    result->average_wall_clock_time = (float)wall_time/capcity;
    result->total_run_time = (float)burst_time;
    
    return true;   
}

#ifdef GRAD_TESTS
bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
    if(ready_queue == NULL || result == NULL) return false;
    
    ProcessControlBlock_t pcb;    
    size_t capacity = dyn_array_size(ready_queue);
    
    float total_latency = 0;
    float total_wall_clock = 0;
    float counter = 0;
    
    while(!dyn_array_empty(ready_queue)){
      
      dyn_array_extract_back(ready_queue, (void*)&pcb);
      
      if(pcb.started == false){
        total_latency += counter;
        pcb.started = 1pcb.started;
      }
      
      for(uint32_t i = 0; i< quantum; i++){
        virtual_cpu(&pcb);
        counter++;
        if(pcb.ermaining_burst_time == 0) break;
      }
      
      if(pcb.remaining_burst_time == 0){
        total_wall_clock += counter;
        continue;
      }
      else{
        dyn_array_push_front(ready_queue, &pcb);
      }
    
    }
    
    result->average_latency_time = total_latency/capacity;
    result->average_wall_clock_time = total_wall_time/capcity;
    result->total_run_time = counter;
    
    return true;  
}
#endif

dyn_array_t *load_process_control_blocks(const char *input_file) 
{
    if(input_file == NULL) return NULL;
    
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
    
    for(uint32_t i = 0; i < length; i++){
      size = read(file_descriptor. &pcb[i].remaining_burst_time, sizeof(unint32_t));
      size = read(file_descriptor. &pcb[i].priority, sizeof(unint32_t));      
    }
    
    dyn_array_t* dynamic_array = dyn_array_import((void*) pcb, sizeof(length), sizeof(ProcessControlBlock_t), NULL);
    
    close(file_descriptor);
    free(pcb);
    
    return dynamic_array;
}

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    // error check params
    if(!ready_queue || !result)
        return false;

    uint32_t size = dyn_array_size(ready_queue);

    // store inital arrival time in priority
    for(uint32_t i = 0; i < size; i++) {
        ProcessControlBlock_t *pcb_ptr = (ProcessControlBlock_t *)dyn_array_at(ready_queue, i);
        pcb_ptr->priority = pcb_ptr->arrival;
    }

    dyn_array_t *dyn_arr_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);

    uint32_t waiting_time = 0;
    uint32_t turnaround_time = 0;
    uint32_t run_time = 0;

    // use arrival calc helper to determine arrival difference and sort
    dyn_array_sort(ready_queue, arrival_calc_helper);
    // initial dyn array queue time
    pcb_queue_sort_by_time_helper(ready_queue, dyn_arr_queue, run_time);

    // while dyn array size has not been capped
    while(dyn_array_size(dyn_arr_queue) != 0) {
        ProcessControlBlock_t pcb;
        dyn_array_extract_front(dyn_arr_queue, &pcb);

        // update variables
        waiting_time += run_time - pcb.arrival;
        // create ready queue size and run time variables
        uint32_t ready_queue_size = dyn_array_size(ready_queue);
        uint32_t current_run_time;

        // if there's remaining space in ready queue
        if(ready_queue_size != 0) {
            uint32_t arrival_n;
            arrival_n = ((ProcessControlBlock_t *)dyn_array_at(ready_queue, dyn_array_size(ready_queue) - 1))->arrival;
            current_run_time = arrival_n - run_time;
        } else
            current_run_time = pcb.remaining_burst_time;

        // increment run time with current run time
        run_time += current_run_time;

        // while pcb has started
        while(current_run_time > 0) {
            virtual_cpu(&pcb);
            current_run_time--;
        }

        // add the pcb arrival to the total run time
        pcb.arrival = run_time;

        if(pcb.remaining_burst_time == 0)
            // check if pcb has completed and calc turnaround time with run time and initial dyn array queue time
            turnaround_time += run_time - pcb.priority;
        else
            dyn_array_push_back(dyn_arr_queue, &pcb);

        pcb_queue_sort_by_time_helper(ready_queue, dyn_arr_queue, run_time);
    }

    // set results
    result->average_waiting_time = (float)waiting_time / size;
    result->average_turnaround_time = (float)turnaround_time / size;
    result->total_run_time = run_time;

    // destroy dyn array queue
    dyn_array_destroy(dyn_arr_queue);

    return true;
}


bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if(!ready_queue || !result)
        return false;

    result->average_waiting_time = 0;
    result->average_turnaround_time = 0;
    result->total_run_time = 0;

    bool extract = false;

    uint32_t size = dyn_array_size(ready_queue);
    uint32_t clock_time = 0;
    uint32_t total_latency = 0;
    uint32_t turnaround_time = 0;
    int arrival_sum = 0;

    ProcessControlBlock_t *pcb_ptr = malloc(sizeof(ProcessControlBlock_t));
    pcb_ptr = (ProcessControlBlock_t *)dyn_array_export(ready_queue);
    dyn_array_t *dyn_arr_queue = dyn_array_create(0, size, NULL);

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
            
            dyn_array_sort(dyn_arr_queue, shortest_burst_time_helper);

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


    result->average_waiting_time = (float)(total_latency - arrival_sum) / size;
    result->average_turnaround_time = (float)(turnaround_time - arrival_sum) / size;
    result->total_run_time = clock_time;
    return true;
}

void pcb_queue_helper(dyn_array_t *ready_queue, dyn_array_t *dyn_arr_queue, uint32_t run_time) {
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

void pcb_queue_sort_by_time_helper(dyn_array_t *ready_queue, dyn_array_t *dyn_arr_queue, uint32_t run_time) {
    
    pcb_queue_helper(ready_queue, dyn_arr_queue, run_time);
    dyn_array_sort(dyn_arr_queue, burst_time_calc_helper);
}

int arrival_calc_helper(const void *pcb1, const void *pcb2) {
    
    uint32_t a = ((ProcessControlBlock_t *)pcb1)->arrival;
    uint32_t b = ((ProcessControlBlock_t *)pcb2)->arrival;
    if(a == b)
        return ((ProcessControlBlock_t *)pcb1)->remaining_burst_time - ((ProcessControlBlock_t *)pcb2)->remaining_burst_time;
    return b - a;
}

int burst_time_calc_helper(const void *pcb1, const void *pcb2) {

    return ((ProcessControlBlock_t *)pcb1)->remaining_burst_time - ((ProcessControlBlock_t *)pcb2)->remaining_burst_time;
}

int shortest_burst_time_helper(const void *pcb1, const void *pcb2)
{
    
    if (((ProcessControlBlock_t *)pcb1)->remaining_burst_time > ((ProcessControlBlock_t *)pcb2)->remaining_burst_time)
        return -1;
    else if (((ProcessControlBlock_t *)pcb1)->remaining_burst_time < ((ProcessControlBlock_t *)pcb2)->remaining_burst_time)
        return 1;
    return 0;
}
