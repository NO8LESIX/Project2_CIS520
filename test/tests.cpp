#include <fcntl.h>
#include <stdio.h>
#include <gtest/gtest.h>
#include <pthread.h>
#include <processing_scheduling.h>
// Using a C library requires extern "C" to prevent function managling
extern "C" 
{
#include <dyn_array.h>
}
#define NUM_PCB 30
#define QUANTUM1 4
#define QUANTUM2 5

unsigned int score;
unsigned int total;

/*
 * LOAD PROCESS CONTROL BLOCKS TEST CASES
 */
TEST (load_process_control_blocks, NullFilePath) {
    dyn_array_t *ready_queue = load_process_control_blocks(NULL);
    ASSERT_EQ(ready_queue, (dyn_array_t*) NULL);
}

TEST (load_process_control_blocks, FileNameIsEmpty) {
    const char *filename = "";
    dyn_array_t *ready_queue = load_process_control_blocks(filename);
    ASSERT_EQ(ready_queue, (dyn_array_t*) NULL);
}
TEST (load_process_control_blocks, FileNameIsNewLine) {
    const char *filename = "\n";
    dyn_array_t *ready_queue = load_process_control_blocks(filename);
    ASSERT_EQ(ready_queue, (dyn_array_t*) NULL);
}
TEST (load_process_control_blocks, FileIsEmpty) {
    const char *filename = "EmptyFile.EMPTY";
    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    close(fd);
    dyn_array_t *ready_queue = load_process_control_blocks(filename);
    ASSERT_EQ(ready_queue, (dyn_array_t*) NULL);
}
TEST (load_process_control_blocks, FilePathIsNull) 
{
    dyn_array_t* ready_queue = load_process_control_blocks (NULL);
    ASSERT_EQ(ready_queue,(dyn_array_t*) NULL);
}
TEST (load_process_control_blocks, FileNotFound) 
{
    dyn_array_t* ready_queue = load_process_control_blocks ("FileNotFound.txt");
    ASSERT_EQ(ready_queue,(dyn_array_t*)NULL);
}
TEST (load_process_control_blocks, FoundFileIsEmptyWithFlags) 
{
    const char* fname = "Empty.File";
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    int flags = O_CREAT | O_TRUNC | O_WRONLY;
    int fd = open(fname, flags, mode);
    close(fd);
    dyn_array_t* ready_queue = load_process_control_blocks (fname);
    ASSERT_EQ(ready_queue,(dyn_array_t*)NULL);
}
TEST (load_process_control_blocks, PCBFileHasBadInput) 
{
    const char* fname = "BadInput.Data";
    uint32_t pcb_num = 10;
    uint32_t pcbs[5][3] = {{1,1,1},{2,2,2},{3,3,3},{4,4,4},{5,5,5}};
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    int flags = O_CREAT | O_TRUNC | O_WRONLY;
    int fd = open(fname, flags, mode);
    write(fd,&pcb_num,sizeof(uint32_t));
    write(fd,pcbs,5 * sizeof(uint32_t)*3);
    close(fd);
    dyn_array_t* ready_queue = load_process_control_blocks (fname);
    ASSERT_EQ(ready_queue,(dyn_array_t*)NULL);
}
/*
 * First Come First Served TEST CASES
 */
TEST (first_come_first_serve, ReadyQueueIsNull) {
    ScheduleResult_t *result = new ScheduleResult_t;
    dyn_array_t *ready_queue = NULL;
    bool res = first_come_first_serve(ready_queue, result);
    ASSERT_EQ(false, res);
    delete result;
}
TEST (first_come_first_serve, ScheduleResultIsNull) {
    ScheduleResult_t *result = NULL;
    dyn_array_t* ready_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    bool res = first_come_first_serve(ready_queue, result);
    ASSERT_EQ(false, res);
    dyn_array_destroy(ready_queue);
}
TEST (first_come_first_serve, HasGoodInputA) 
{
    ScheduleResult_t *result = new ScheduleResult_t;
    dyn_array_t* ready_queue = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
    memset(result,0,sizeof(ScheduleResult_t));
    ProcessControlBlock_t data[3] = 
    {
        [0] = {24,2,0,0},
        [1] = {3,3,0,0},
        [2] = {3,1,0,0}
    };
    dyn_array_push_back(ready_queue,&data[2]);
    dyn_array_push_back(ready_queue,&data[1]);
    dyn_array_push_back(ready_queue,&data[0]);	
    bool res = first_come_first_serve (ready_queue,result);	
    ASSERT_EQ(true,res);
    float answers[3] = {27,17,30};
    //assert expected is what is stored in result and clean up
    ASSERT_EQ(answers[0],result->average_turnaround_time);
    ASSERT_EQ(answers[1],result->average_waiting_time);
    ASSERT_EQ(answers[2],result->total_run_time);
    dyn_array_destroy(ready_queue);
    delete result;
}
TEST (first_come_first_serve, HasGoodInputB) 
{
    ScheduleResult_t *result = new ScheduleResult_t;
    dyn_array_t* ready_queue = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
    memset(result,0,sizeof(ScheduleResult_t));
    ProcessControlBlock_t data[4] = 
    {
        [0] = {6,3,0,0},
        [1] = {8,2,0,0},
        [2] = {7,4,0,0},
        [3] = {3,1,0,0},
    };
    dyn_array_push_back(ready_queue,&data[3]);
    dyn_array_push_back(ready_queue,&data[2]);
    dyn_array_push_back(ready_queue,&data[1]);		
    dyn_array_push_back(ready_queue,&data[0]);	
    bool res = first_come_first_serve (ready_queue,result);	
    ASSERT_EQ(true,res);
    float answers[3] = {16.25,10.25,24};
    //assert expected is what is stored in result and clean up
    ASSERT_EQ(answers[0],result->average_turnaround_time);
    ASSERT_EQ(answers[1],result->average_waiting_time);
    ASSERT_EQ(answers[2],result->total_run_time);
    dyn_array_destroy(ready_queue);
    delete result;
}
/*
 * Shortest Job First TEST CASES
 */
TEST (shortest_job_first, ReadyQueueIsNull) {
    ScheduleResult_t *result = new ScheduleResult_t;
    dyn_array_t *ready_queue = NULL;
    bool res = shortest_job_first(ready_queue, result);
    ASSERT_EQ(false, res);
    delete result;
}
TEST (shortest_job_first, ScheduleResultIsNull) 
{
    ScheduleResult_t *result = NULL;
    dyn_array_t* ready_queue = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
    bool res = shortest_job_first (ready_queue,result);
    ASSERT_EQ(false,res);
    dyn_array_destroy(ready_queue);
}
TEST (shortest_job_first, GoodInputA) 
{
    ScheduleResult_t *result = new ScheduleResult_t;
    dyn_array_t* ready_queue = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
    memset(result,0,sizeof(ScheduleResult_t));
    ProcessControlBlock_t data[4] = 
    {
        [0] = {25,2,0,0},
        [1] = {2,3,1,0},
        [2] = {4,1,2,0},
        [3] = {1,4,3,0},
    };
    dyn_array_push_back(ready_queue,&data[3]);
    dyn_array_push_back(ready_queue,&data[2]);
    dyn_array_push_back(ready_queue,&data[1]);
    dyn_array_push_back(ready_queue,&data[0]);	
    bool res = shortest_job_first (ready_queue,result);	
    ASSERT_EQ(true,res);
    float answers[3] = {26.25,18.25,32};
    //assert expected is what is stored in result and clean up
    ASSERT_EQ(answers[0],result->average_turnaround_time);
    ASSERT_EQ(answers[1],result->average_waiting_time);
    ASSERT_EQ(answers[2],result->total_run_time);
    dyn_array_destroy(ready_queue);
    delete result;
}
/*
 * * Shortest Remaining Time First  TEST CASES
 */
TEST (shortest_remaining_time_first, ReadyQueueIsNull) {
    ScheduleResult_t *result = new ScheduleResult_t;
    dyn_array_t *ready_queue = NULL;
    bool res = shortest_remaining_time_first(ready_queue, result);
    ASSERT_EQ(false, res);
    delete result;
}
TEST (shortest_remaining_time_first, ScheduleResultIsNull) {
    ScheduleResult_t *result = NULL;
    dyn_array_t *ready_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    bool res = shortest_remaining_time_first(ready_queue, result);
    ASSERT_EQ(false, res);
    dyn_array_destroy(ready_queue);
}
TEST (shortest_remaining_time_first, GoodInputA) 
{
    ScheduleResult_t *result = new ScheduleResult_t;
    dyn_array_t* ready_queue = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
    memset(result,0,sizeof(ScheduleResult_t));
    ProcessControlBlock_t data[4] = 
    {
        [0] = {25,2,0,0},
        [1] = {2,3,1,0},
        [2] = {4,1,2,0},
        [3] = {1,4,3,0},
    };
    dyn_array_push_back(ready_queue,&data[3]);
    dyn_array_push_back(ready_queue,&data[2]);
    dyn_array_push_back(ready_queue,&data[1]);
    dyn_array_push_back(ready_queue,&data[0]);	
    bool res = shortest_remaining_time_first (ready_queue,result);	
    ASSERT_EQ(true,res);
    float answers[3] = {10.25,2.25,32};
    //assert expected is what is stored in result and clean up
    ASSERT_EQ(answers[0],result->average_turnaround_time);
    ASSERT_EQ(answers[1],result->average_waiting_time);
    ASSERT_EQ(answers[2],result->total_run_time);
    dyn_array_destroy(ready_queue);
    delete result;
}
/*
 * ROUND ROBIN TEST CASES
 */
TEST (round_robin, ReadyQueueIsNull) {
    ScheduleResult_t *result = new ScheduleResult_t;
    dyn_array_t *ready_queue = NULL;
    bool res = round_robin(ready_queue, result, QUANTUM1);
    ASSERT_EQ(false, res);
    dyn_array_destroy(ready_queue);
}

TEST (round_robin, ScheduleResultIsNull) {
    ScheduleResult_t *result = NULL;
    dyn_array_t *ready_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    bool res = round_robin(ready_queue, result, QUANTUM1);
    ASSERT_EQ(false, res);
    dyn_array_destroy(ready_queue);
}
TEST (round_robin, GoodInputA) 
{
    ScheduleResult_t *result = new ScheduleResult_t;
    dyn_array_t* ready_queue = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
    memset(result,0,sizeof(ScheduleResult_t));
    ProcessControlBlock_t data[3] = 
    {
        [0] = {24,4,0,0},
        [1] = {3,2,0,0},
        [2] = {3,1,0,0}
    };
    dyn_array_push_back(ready_queue,&data[2]);
    dyn_array_push_back(ready_queue,&data[1]);
    dyn_array_push_back(ready_queue,&data[0]);	
    bool res = round_robin (ready_queue,result,QUANTUM1);	
    ASSERT_EQ(true,res);
    float answers[3] = {15.666667,5.666667,30};
    //assert expected is what is stored in result and clean up
    ASSERT_FLOAT_EQ(answers[0],result->average_turnaround_time);
    ASSERT_FLOAT_EQ(answers[1],result->average_waiting_time);
    ASSERT_EQ(answers[2],result->total_run_time);
    dyn_array_destroy(ready_queue);
    delete result;
}
TEST (round_robin, GoodInputB) 
{
    ScheduleResult_t *result = new ScheduleResult_t;
    dyn_array_t* ready_queue = dyn_array_create(0,sizeof(ProcessControlBlock_t),NULL);
    memset(result,0,sizeof(ScheduleResult_t));
    ProcessControlBlock_t data[4] = 
    {
        [0] = {20,1,0,0},
        [1] = {5,2,0,0},
        [2] = {6,3,0,0},
        [3] = {11,4,14,0}
    };
    dyn_array_push_back(ready_queue,&data[3]);
    dyn_array_push_back(ready_queue,&data[2]);
    dyn_array_push_back(ready_queue,&data[1]);     
    dyn_array_push_back(ready_queue,&data[0]); 
    bool res = round_robin (ready_queue,result,QUANTUM2);  
    ASSERT_EQ(true,res);
    float answers[3] = {26.25,15.75,42};
    //assert expected is what is stored in result and clean up
    ASSERT_FLOAT_EQ(answers[0],result->average_turnaround_time);
    ASSERT_EQ(answers[1],result->average_waiting_time);
    ASSERT_EQ(answers[2],result->total_run_time);
    dyn_array_destroy(ready_queue);
    delete result;
}
//came with the tests by default so just left it in/not touching it
class GradeEnvironment : public testing::Environment 
{
    public:
        virtual void SetUp() 
        {
            score = 0;
            total = 190;
        }

        virtual void TearDown() 
        {
            ::testing::Test::RecordProperty("points_given", score);
            ::testing::Test::RecordProperty("points_total", total);
            std::cout << "SCORE: " << score << '/' << total << std::endl;
        }
};

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new GradeEnvironment);
    return RUN_ALL_TESTS();
}