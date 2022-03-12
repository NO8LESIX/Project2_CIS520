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

//FCFS tests

/*
*  ensure null dyn_array is not processed.
*/
TEST(first_come_first_serve, ReadyQueueIsNull){
  ScheduleResult_t *sr = new ScheduleResult_t;
	dyn_array_t* pcbs = NULL;
	bool res = first_come_first_serve (pcbs,sr);
	EXPECT_EQ(false,res);
	delete sr;
}

TEST(first_come_first_serve, ScheduleResultIsNull){
  ScheduleResult_t *sr = NULL;
	dyn_array_t* pcbs = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
	bool res = first_come_first_serve (pcbs,sr);
	EXPECT_EQ(false,res);
	delete sr;
}

TEST (first_come_first_serve, goodInputA) 
{
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
    dyn_array_destroy(pcbs);
    delete sr;
}

TEST (first_come_first_serve, goodInputB) 
{
    ScheduleResult_t *sr = new ScheduleResult_t;
    dyn_array_t* pcbs = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
    memset(sr,0,sizeof(ScheduleResult_t));
    // add PCBs now
    ProcessControlBlock_t data[4] = 
    {
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
    dyn_array_destroy(pcbs);
    delete sr;
}

//priority tests



//End Priority tests

//round_robin tests



//end round_robin_tests

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
