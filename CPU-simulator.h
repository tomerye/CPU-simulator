

#ifndef _CPU_SIMULATOR_H
#define _CPU_SIMULATOR_H

#include <iostream>
#include <vector>
#include <string>
#include <map>

//struct that contain the ini file parameters
typedef struct ConfigurationStruct_
{
	int addsub_delay;
	int mul_delay;
	int div_delay;
	int instruction_q_depth;
	int addsub_rs;
	int muldiv_rs;
	int load_q_depth;
	int reorder_buffer;
	int ghr_width;
	int two_threads_enabled;
	int l1_block_size;
	int l1_access_delay;
	int l1_cache_size;
	int l2_block_size;
	int l2_access_delay;
	int l2_cache_size;
	int mem_access_delay;

} ConfigurationStruct;

#define	NUMBER_OF_REGISTERS 32

#define MEMORY_SIZE 0x00FF0000

#define MAX_LINE_SIZE 50



//write registers value to file
int WriteRegisterDumpToFile(char *file_name);
//write memory value to file
int WriteMemoryDumpToFile(char *file_name);
//read each line from the cmd file and send it to ParseLine function to do the actual parsing
int ParseCMDfile(char *file_name);
//this function is from the library we used to parse the config file
static int handler(void* user, const char* section, const char* name,const char* value);
//this function parse the cmd line and store the output in global vector
void ParseLine(char *lineBuffer);
//help function for the line parsing
int GetRegNumberFromString(std::string reg);
//convert int to string (include 0) 
int MyAtoi(std::string str);
////help function for the line parsing
int GetOffset(std::string s);
//do the simulation
void StartSimulator();
////read the file that contain the initicial memmory
void ReadMemInitFile(char *file_name);
//write excecution time to file
void WriteExceutionTime(char *file_name);
//write the totlal insctruction that excecute during the simulation
void WriteInstructionCount(char *file_name);




#endif

