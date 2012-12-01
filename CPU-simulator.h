

#ifndef _CPU_SIMULATOR_H
#define _CPU_SIMULATOR_H

#include <iostream>
#include <vector>
#include <string>
#include <map>


//struct that contain the ini file parameters
typedef struct ConfigurationStruk_
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

#define MEMORY_SIZE 65536


#define MAX_LINE_SIZE 50

void ReadInitMemory(char *file_name);
int WriteRegisterDumpToFile(char *file_name);
int WriteMemoryDumpToFile(char *file_name);
int WriteExcTimeToFile(char *file_name, int time);
int WriteExcCommandNumberToFile(char *file_name, int command_number);
int ParseCMDfile(char *file_name);
static int handler(void* user, const char* section, const char* name,const char* value);
void ParseLine(char *lineBuffer);
int GetRegNumberFromString(std::string reg);
template< typename T >
T MyAtoi(std::string str);
int GetOffset(std::string s);
void StartSimulator();
void ReadMemInitFile(char *file_name);
void WriteExceutionTime(char *file_name);

void WriteInstructionCount(char *file_name);

class MyClass
{
public:
	void testClass(){std::cout << "teeeet";}
};



#endif

