
// CPU-simulator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "CPU-simulator.h"
#include "ini.h"
#include <string.h>


std::vector<std::vector<std::string> > commands_vector;
int instruction_counter;
std::map<std::string,int> lables_map;

int pc=0;
int ram[MEMORY_SIZE];
int reg[NUMBER_OF_REGISTERS];


int _tmain(int argc, char* argv[])
{

	ConfigurationStruct configuration;

	int tmp=10;
	int i;


	if(argc!=2)
	{
		printf("Wronge command line arguments number!\n");
		exit(1);
	}

	if (ini_parse(argv[1], handler, &configuration) < 0) {
		printf("Can't load '%s' file\n",argv[1]);
		exit(1);
	}

	ReadMemInitFile(argv[3]);

	ParseCMDfile(argv[1]);

	StartSimulator();

	WriteMemoryDumpToFile(argv[5]);

	WriteRegisterDumpToFile(argv[4]);



	//	printf("%d",configuration.l1_cache_size);



	return 0;


}


//for string $1 return int 1
int GetRegNumberFromString(std::string reg)
{
	if(reg.at(0)!='$')	
	{
		printf("error in instruction at function GetRegNumberFromString pc=%d\n",pc);
		exit(1);
	}

	return atoi(reg.c_str()+1);//convert to int after the $ sign
}


int MyAtoi(std::string s)
{
	int res=atoi(s.c_str());
	if(res)
	{
		printf("error in instruction at function MyAtoi pc=%d\n",pc);
		exit(1);
	}

	return res;
}


int GetOffset(std::string s)
{
	std::string tmp=s.substr(s.find_first_of("(",0),s.find_first_of(")"));
	return MyAtoi(tmp);
}


void StartSimulator()
{
	std::vector<std::string> current_instruction;
	int r0,r1,r2,res;
	while(1)
	{
		current_instruction=commands_vector[pc];
		if(current_instruction[1]=="halt")
			return;
		if(current_instruction[1]=="j")
		{
			pc=lables_map[current_instruction[2]];
			continue;
		}
		if(current_instruction[1]=="add")
		{
			r0=GetRegNumberFromString(current_instruction[2]);
			r1=GetRegNumberFromString(current_instruction[3]);
			r2=GetRegNumberFromString(current_instruction[4]);
			reg[r0]=reg[r1]+reg[r2];
			pc++;
			continue;
		}
		if(current_instruction[1]=="sub")
		{
			r0=GetRegNumberFromString(current_instruction[2]);
			r1=GetRegNumberFromString(current_instruction[3]);
			r2=GetRegNumberFromString(current_instruction[4]);
			reg[r0]=reg[r1]-reg[r2];
			pc++;
			continue;
		}
		if(current_instruction[1]=="mul")
		{
			r0=GetRegNumberFromString(current_instruction[2]);
			r1=GetRegNumberFromString(current_instruction[3]);
			r2=GetRegNumberFromString(current_instruction[4]);
			reg[r0]=reg[r1]*reg[r2];
			pc++;
			continue;
		}
		if(current_instruction[1]=="div")
		{
			r0=GetRegNumberFromString(current_instruction[2]);
			r1=GetRegNumberFromString(current_instruction[3]);
			r2=GetRegNumberFromString(current_instruction[4]);
			reg[r0]=reg[r1]/reg[r2];
			pc++;
			continue;
		}
		if(current_instruction[1]=="slt")
		{
			r0=GetRegNumberFromString(current_instruction[2]);
			r1=GetRegNumberFromString(current_instruction[3]);
			r2=GetRegNumberFromString(current_instruction[4]);
			reg[r0]=reg[r1]<reg[r2]?1:0;
			pc++;
			continue;
		}
		if(current_instruction[1]=="addi")
		{
			r0=GetRegNumberFromString(current_instruction[2]);
			r1=GetRegNumberFromString(current_instruction[3]);
			res=MyAtoi(current_instruction[4]);
			reg[r0]=reg[r1]+res;
			pc++;
			continue;
		}
		if(current_instruction[1]=="subi")
		{
			r0=GetRegNumberFromString(current_instruction[2]);
			r1=GetRegNumberFromString(current_instruction[3]);
			res=MyAtoi(current_instruction[4]);
			reg[r0]=reg[r1]-res;
			pc++;
			continue;
		}
		if(current_instruction[1]=="slti")
		{
			r0=GetRegNumberFromString(current_instruction[2]);
			r1=GetRegNumberFromString(current_instruction[3]);
			res=MyAtoi(current_instruction[4]);
			reg[r0]=reg[r1]<res?1:0;
			pc++;
			continue;
		}
		if(current_instruction[1]=="beq")
		{
			r0=GetRegNumberFromString(current_instruction[2]);
			r1=GetRegNumberFromString(current_instruction[3]);

			if(reg[r0]==reg[r1])
			{
				pc=lables_map[current_instruction[4]];
				continue;
			}
			pc++;
			continue;
		}
		if(current_instruction[1]=="bne")
		{
			r0=GetRegNumberFromString(current_instruction[2]);
			r1=GetRegNumberFromString(current_instruction[3]);

			if(reg[r0]!=reg[r1])
			{
				pc=lables_map[current_instruction[4]];
				continue;
			}
			pc++;
			continue;
		}
		if(current_instruction[1]=="lw")
		{
			r0=GetRegNumberFromString(current_instruction[2]);
			res=GetOffset(current_instruction[3]);
			r1=GetRegNumberFromString(current_instruction[3].substr(current_instruction[3].find_first_of(")"),std::string::npos));
			reg[r0]=ram[reg[r1]+res];
			pc++;
			continue;
		}
		if(current_instruction[1]=="sw")
		{
			r0=GetRegNumberFromString(current_instruction[2]);
			res=GetOffset(current_instruction[3]);
			r1=GetRegNumberFromString(current_instruction[3].substr(current_instruction[3].find_first_of(")"),std::string::npos));
			ram[reg[r1]+res]=reg[r0];
			pc++;
			continue;
		}

		printf("unknown instruction\n");
		exit(1);
	}
}



//private function used by ini_parse
static int handler(void* user, const char* section, const char* name,
	const char* value)
{
	ConfigurationStruct* pconfig = (ConfigurationStruct*)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (MATCH("", "addsub_delay"))
	{
		pconfig->addsub_delay = atoi(value);
	}
	else if (MATCH("", "mul_delay"))
	{
		pconfig->mul_delay = atoi(value);
	}
	else if (MATCH("", "div_delay"))
	{
		pconfig->div_delay = atoi(value);
	}
	else if (MATCH("", "instruction_q_depth"))
	{
		pconfig->instruction_q_depth = atoi(value);
	}
	else if (MATCH("", "addsub_rs"))
	{
		pconfig->addsub_rs = atoi(value);
	}
	else if (MATCH("", "muldiv_rs"))
	{
		pconfig->muldiv_rs = atoi(value);
	}
	else if (MATCH("", "load_q_depth"))
	{
		pconfig->load_q_depth = atoi(value);
	}
	else if (MATCH("", "reorder_buffer"))
	{
		pconfig->reorder_buffer = atoi(value);
	}
	else if (MATCH("", "ghr_width"))
	{
		pconfig->ghr_width = atoi(value);
	}
	else if (MATCH("", "two_threads_enabled"))
	{
		pconfig->two_threads_enabled = atoi(value);
	}
	else if (MATCH("", "l1_block_size"))
	{
		pconfig->l1_block_size = atoi(value);
	}
	else if (MATCH("", "l1_access_delay"))
	{
		pconfig->l1_access_delay = atoi(value);
	}
	else if (MATCH("", "l1_cache_size"))
	{
		pconfig->l1_cache_size = atoi(value);
	}
	else if (MATCH("", "l2_block_size"))
	{
		pconfig->l2_block_size = atoi(value);
	}
	else if (MATCH("", "l2_access_delay"))
	{
		pconfig->l2_access_delay = atoi(value);
	}
	else if (MATCH("", "l2_cache_size"))
	{
		pconfig->l2_cache_size = atoi(value);
	}
	else if (MATCH("", "mem_access_delay"))
	{
		pconfig->mem_access_delay = atoi(value);
	}
	else 
	{
		return 0;  /* unknown section/name, error */
	}

	return 1;
}


int WriteRegisterDumpToFile(char *file_name)
{
	FILE *file=fopen(file_name,"w");
	int i;

	if (file==NULL)
	{
		printf("error in write registery dump file\n %s %s ",__FILE__,__LINE__);
		return 0;
	}

	for(i=0;i<NUMBER_OF_REGISTERS;i++)
	{
		fprintf(file,"$%d %d\n",i,reg[i]);
	}

	fclose(file);
	return 1;
}


int WriteMemoryDumpToFile(char *file_name)
{
	FILE *file=fopen(file_name,"w");
	int i;

	if (file==NULL)
	{
		printf("error in write memory dump file\n %s %s ",__FILE__,__LINE__);
		return 0;
	}

	for(i=0;i<MEMORY_SIZE;i++)
	{
		if(i%8==0)
			fprintf(file,"%X ",ram[i]);
		else
			fprintf(file,"\n");
	}

	fclose(file);
	return 1;
}


void ReadMemInitFile(char *file_name)
{
	FILE *file=fopen(file_name,"w");
	char lineBuffer[MAX_LINE_SIZE];
	int status;
	int x[8];
	int i=0;

	if (file==NULL)
	{
		printf("error in write CMD file\n %s %s ",__FILE__,__LINE__);
		exit(1);
	}


	while(fgets(lineBuffer, MAX_LINE_SIZE, file)!= NULL)
	{
		status = sscanf(lineBuffer, "%02x %02x %02x %02x %02x %02x %02x %02x\r\n",
			&x[0], &x[1], &x[2], &x[3], &x[4], &x[5], &x[6], &x[7]);
		if(status!=8)
		{
			printf("error in parsing the init mem file\n");
			exit(1);
		}

		ram[i]=x[0]+x[1]+x[2]+x[3];
		ram[i+1]=x[4]+x[5]+x[6]+x[7];
		i+=2;
	}

}

int ParseCMDfile(char *file_name)
{
	FILE *file=fopen(file_name,"w");
	char lineBuffer[MAX_LINE_SIZE];

	if (file==NULL)
	{
		printf("error in write CMD file\n %s %s ",__FILE__,__LINE__);
		return 0;
	}

	while ( fgets ( lineBuffer, sizeof lineBuffer, file ) != NULL ) 
	{
		ParseLine(lineBuffer);
	}

	fclose(file);
	return 1;
}




void ParseLine(char *lineBuffer)
{
	std::string tmp(lineBuffer);
	std::string lable,cmd,rs,r0,r1,jump_lable;
	std::vector<std::string> instruction;
	int result;
	size_t start=0,end=0;

	if(tmp.find_first_of(':',0)==std::string::npos)//regular line
	{
		instruction.push_back("#");
	}
	else// lable line
	{
		end=tmp.find_first_of(":",0);
		cmd=tmp.substr(start,end-start);
		instruction.push_back(cmd);
		lables_map[cmd]=instruction_counter;
	}
	start=tmp.find_first_not_of(" \t",0);
	end=tmp.find_first_of(" \,\t\r\n",start);
	while(start!=std::string::npos){
		cmd=tmp.substr(start,end-start);
		instruction.push_back(cmd);
		start=tmp.find_first_not_of(" \,\t",end);
		end=tmp.find_first_of(" \,\t\r\n",start);
	}
	instruction_counter++;
	commands_vector.push_back(instruction);
}


