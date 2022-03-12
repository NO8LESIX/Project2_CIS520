#include <fcntl.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include <pthread.h>
#include "../include/processing_scheduling.h"

// Using a C library requires extern "C" to prevent function managling
extern "C" 
{
#include <dyn_array.h>
}

#define NUM_PCB 30
#define QUANTUM 5 // Used for Robin Round for process as the run time limit

unsigned int score;
unsigned int total;

class GradeEnvironment : public testing::Environment 
{
    public:
        virtual void SetUp() 
        {
            score = 0;
            total = 210;
        }

        virtual void TearDown()
        {
            ::testing::Test::RecordProperty("points_given", score);
            ::testing::Test::RecordProperty("points_total", total);
            std::cout << "SCORE: " << score << '/' << total << std::endl;
        }
};

//Start FCFS tests
/*
*  ensure null dyn_array is not processed.
*/
TEST (first_come_first_serve, FCFS:ReadyQueueIsNull) {
    ScheduleResult_t *sr = new ScheduleResult_t;
	  dyn_array_t* pcbs = NULL;
	  bool res = first_come_first_serve (pcbs,sr);
	  EXPECT_EQ(false,res);
     
    //garbage collection
	  delete sr;
}

 /*
  *
  */
TEST (first_come_first_serve, FCFS:ScheduleResultIsNull) {
    ScheduleResult_t *sr = NULL;
	  dyn_array_t* pcbs = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
	  bool res = first_come_first_serve (pcbs,sr);
	  EXPECT_EQ(false,res);
     
    //garbage collection
	  delete sr;
}

 /*
  *
  */
TEST (first_come_first_serve, FCFS:GoodInputA) {
    ScheduleResult_t *sr = new ScheduleResult_t;
    dyn_array_t* pcbs = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
    memset(sr,0,sizeof(ScheduleResult_t));
    // add PCBs now
    ProcessControlBlock_t data[3] = 
    {
        [0] = {24,2,0,0},
        [1] = {3,3,0,0},
        [2] = {3,1,0,0}
    };
    // back loading dyn_array, pull from the back
    dyn_array_push_back(pcbs,&data[2]);
    dyn_array_push_back(pcbs,&data[1]);
    dyn_array_push_back(pcbs,&data[0]);	
    bool res = first_come_first_serve (pcbs,sr);	
    ASSERT_EQ(true,res);
    float answers[3] = {27,17,30};
    ASSERT_EQ(answers[0],sr->average_turnaround_time);
    ASSERT_EQ(answers[1],sr->average_waiting_time);
    ASSERT_EQ(answers[2],sr->total_run_time);
    
    //garbage collection
    dyn_array_destroy(pcbs);
    delete sr;
}

 /*
  *
  */
TEST (first_come_first_serve, FCFS:GoodInputB) {
    ScheduleResult_t *sr = new ScheduleResult_t;
    dyn_array_t* pcbs = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
    memset(sr,0,sizeof(ScheduleResult_t));
    // add PCBs now
    ProcessControlBlock_t data[4] = {
      [0] = {6,3,0,0},
      [1] = {8,2,0,0},
      [2] = {7,4,0,0},
      [3] = {3,1,0,0},
    };
    // back loading dyn_array, pull from the back
    dyn_array_push_back(pcbs,&data[3]);
    dyn_array_push_back(pcbs,&data[2]);
    dyn_array_push_back(pcbs,&data[1]);		
    dyn_array_push_back(pcbs,&data[0]);	
    bool res = first_come_first_serve (pcbs,sr);	
    ASSERT_EQ(true,res);
    float answers[3] = {16.25,10.25,24};
    ASSERT_EQ(answers[0],sr->average_turnaround_time);
    ASSERT_EQ(answers[1],sr->average_waiting_time);
    ASSERT_EQ(answers[2],sr->total_run_time);
    
    //garbage collection    
    dyn_array_destroy(pcbs);
    delete sr;
}//End FCFS tests

//Start load_process_control_blocks tests
 /*
  *
  */
TEST (load_process_control_blocks, LPCB:FilePathIsNull) {
	  dyn_array_t* pcbs = load_process_control_blocks (NULL);
    ASSERT_EQ(pcbs,(dyn_array_t*) NULL);
}

 /*
  *
  */
TEST (load_process_control_blocks, LPCB:FileNotFound) {
    dyn_array_t* pcbs = load_process_control_blocks ("IDONTEXIST.txt");
	  ASSERT_EQ(pcbs,(dyn_array_t*)NULL);
}

 /*
  *
  */
TEST (load_process_control_blocks, LPCB:FileIsEmpty) {
	  const char* fname = "EMPTYFILE.txt";
	  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
	  int flags = O_CREAT | O_TRUNC | O_WRONLY;
	  int fd = open(fname, flags, mode);
	  close(fd);
	  dyn_array_t* pcbs = load_process_control_blocks (fname);
	  ASSERT_EQ(pcbs,(dyn_array_t*)NULL);
}

 /*
  *
  */
TEST (load_process_control_blocks, LPCB:FoundFileHasIncorrectPCB) {
  	const char* fname = "PCBICORRECT.txt";
  	uint32_t pcb_num = 10;
  	uint32_t pcbs[5][2] = {{1,1},{2,2},{3,3},{4,4},{5,5}};
  	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
  	int flags = O_CREAT | O_TRUNC | O_WRONLY;
  	int fd = open(fname, flags, mode);
  	write(fd,&pcb_num,sizeof(uint32_t));
  	write(fd,pcbs,5 * sizeof(uint32_t)*2);
  	close(fd);
  	dyn_array_t* da = load_process_control_blocks (fname);
  	ASSERT_EQ(da,(dyn_array_t*)NULL);
}

 /*
  *
  */
TEST (load_process_control_blocks, LPCB:FoundFileFull) {
  	const char* fname = "../test/PCBs.bin";
  	uint32_t pcb_num = NUM_PCB;
  	uint32_t pcbs[NUM_PCB][2];
  	for (uint32_t p = 0; p < pcb_num; ++p) {
  		pcbs[p][0] = rand() % 15 + 1;
  		pcbs[p][1] = p;
  	}
  	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
  	int flags = O_CREAT | O_TRUNC | O_WRONLY;
  	int fd = open(fname, flags, mode);
  	write(fd,&pcb_num,sizeof(uint32_t));
  	write(fd,pcbs,pcb_num * sizeof(uint32_t)*2);
  	close(fd);
  	dyn_array_t* da = load_process_control_blocks (fname);
  	ASSERT_NE(da, (dyn_array_t*) NULL);
  	for (size_t i = 0; i < dyn_array_size(da); ++i) {
  		ProcessControlBlock_t * pPCB = (ProcessControlBlock_t *)dyn_array_at(da, i);
                   EXPECT_EQ(pPCB->remaining_burst_time, pcbs[i][0]);
                   EXPECT_EQ(pPCB->priority, pcbs[i][1]);
  	}
   
    //garbage collection
  	dyn_array_destroy(da);
}//End load_process_control_blocks tests


//Start priority tests
 /*
  *
  */
TEST (priority, Priority:ProcessControlBlockIsNull) {
    ScheduleResult_t *sr = new ScheduleResult_t;
    dyn_array_t* pcbs = NULL;
    bool res = priority (pcbs,sr);
    EXPECT_EQ(false,res);
    
    //garbage collection
    delete sr;
}

 /*
  *
  */
TEST (priority, Priority:ScheduleResultIsNull) {
    ScheduleResult_t *sr = NULL;
    dyn_array_t* pcbs = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
    bool res = priority (pcbs,sr);
    EXPECT_EQ(false,res);
    
    //garbage collection
    dyn_array_destroy(pcbs);
}

 /*
  *
  */
TEST (priority, Priority:GoodInputA) {
    ScheduleResult_t *sr = new ScheduleResult_t;
    dyn_array_t* pcbs = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
    memset(sr,0,sizeof(ScheduleResult_t));
    // add PCBs now
    ProcessControlBlock_t data[3] = {
      [0] = {24,3,0},
      [1] = {3,1,0},
      [2] = {3,2,0}
    };
    // back loading dyn_array, pull from the back
    dyn_array_push_back(pcbs,&data[2]);
    dyn_array_push_back(pcbs,&data[1]);
    dyn_array_push_back(pcbs,&data[0]);
    bool res = priority (pcbs,sr);
    ASSERT_EQ(true,res);
    float answers[3] = {13,3,30};
    EXPECT_EQ(answers[0],sr->average_waiting_time);
    EXPECT_EQ(answers[1],sr->average_turnaround_time);
    EXPECT_EQ(answers[2],sr->total_run_time);
    
    //garbage collection
    dyn_array_destroy(pcbs);
    delete sr;
}

 /*
  *
  */
TEST (priority, Priority:GoodInputB) {
    ScheduleResult_t *sr = new ScheduleResult_t;
    dyn_array_t* pcbs = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
    memset(sr,0,sizeof(ScheduleResult_t));
    // add PCBs now
	  ProcessControlBlock_t data[5] = {
      [0] = {10,3,0},
      [1] = {1,1,0},
      [2] = {2,4,0},
      [3] = {1,5,0},
			[4] = {5,2,0},
    };
    // back loading dyn_array, pull from the back
    dyn_array_push_back(pcbs,&data[4]);
    dyn_array_push_back(pcbs,&data[3]);
    dyn_array_push_back(pcbs,&data[2]);
    dyn_array_push_back(pcbs,&data[1]);
    dyn_array_push_back(pcbs,&data[0]);
    bool res = priority (pcbs,sr);
    ASSERT_EQ(true,res);
    float answers[3] = {12,8.2,19};
    EXPECT_EQ(answers[0],sr->average_waiting_time);
    EXPECT_EQ(answers[1],sr->average_turnaround_time);
    EXPECT_EQ(answers[2],sr->total_run_time);
    
    //garbage collection
    dyn_array_destroy(pcbs);
    delete sr;
}//End Priority tests

//Start round_robin tests
 /*
  * Expect no result if ready queue is null
  */
TEST (round_robin, RR:ReadyQueueIsNull) {
  	ScheduleResult_t *sr = new ScheduleResult_t;
  	dyn_array_t* pcbs = NULL;
  	bool res = round_robin (pcbs,sr,QUANTUM);
  	EXPECT_EQ(false,res);
   
    //garbage collection
  	delete sr;
}

 /*
  *  Expect rr to not execute with a null schedule
  */
TEST (round_robin, RR:ScheduleResultIsNull) {
  	ScheduleResult_t *sr = NULL;
  	dyn_array_t* pcbs = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
  	bool res = round_robin (pcbs,sr,QUANTUM);
  	EXPECT_EQ(false,res);
   
    //garbage collection
  	dyn_array_destroy(pcbs);
}

 /*
  *  Evaluate 
  */
TEST (round_robin, RR:GoodInputA) {
  	ScheduleResult_t *sr = new ScheduleResult_t;
  	dyn_array_t* pcbs = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
  	memset(sr,0,sizeof(ScheduleResult_t));
  	// add PCBs now
  	ProcessControlBlock_t data[3] = {
  			[0] = {24,0,0},
  			[1] = {3,0,0},
  			[2] = {3,0,0}
  	};
  	// back loading dyn_array, pull from the back
  	dyn_array_push_back(pcbs,&data[2]);
  	dyn_array_push_back(pcbs,&data[1]);
  	dyn_array_push_back(pcbs,&data[0]);	
  	bool res = round_robin (pcbs,sr,QUANTUM);	
  	ASSERT_EQ(true,res);
  	float answers[3] = {15.666667,5.666667,30};
  	EXPECT_FLOAT_EQ(answers[0],sr->average_waiting_time);
  	EXPECT_FLOAT_EQ(answers[1],sr->average_turnaround_time);
  	EXPECT_EQ(answers[2],sr->total_run_time);
   
    //garbage collection
  	dyn_array_destroy(pcbs);
  	delete sr;
}

 /*
  *
  */
TEST (round_robin, RR:GoodInputB) {
  	ScheduleResult_t *sr = new ScheduleResult_t;
  	dyn_array_t* pcbs = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
  	memset(sr,0,sizeof(ScheduleResult_t));
  	// add PCBs now
  	ProcessControlBlock_t data[3] = {
  			[0] = {20,0,0},
  			[1] = {5,0,0},
  			[2] = {6,0,0}
  	};
  	// back loading dyn_array, pull from the back
  	dyn_array_push_back(pcbs,&data[2]);
  	dyn_array_push_back(pcbs,&data[1]);		
  	dyn_array_push_back(pcbs,&data[0]);	
  	bool res = round_robin (pcbs,sr,QUANTUM);	
  	ASSERT_EQ(true,res);
  	float answers[3] = {22.333334,12,31};
  	EXPECT_FLOAT_EQ(answers[0],sr->average_waiting_time);
  	EXPECT_EQ(answers[1],sr->average_turnaround_time);
  	EXPECT_EQ(answers[2],sr->total_run_time);
   
    //garbage collection
  	dyn_array_destroy(pcbs);
  	delete sr;
}//End round_robin_tests

// shortest_job_first tests



//End  tests

//shortest_remaining_time_first tests



//End shortest_remaining_time_first tests


int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new GradeEnvironment);
    return RUN_ALL_TESTS();
}
