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
    UNUSED(ready_queue);
    UNUSED(result); 
    return false;
}

bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    UNUSED(ready_queue);
    UNUSED(result);
    return false;   
}