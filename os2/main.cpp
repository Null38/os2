#include <iostream>
#include <thread>
#include <mutex>
#include <windows.h>
#include <string>
#include <regex>
#include <vector>

using namespace std;

#define THRESHOLD 10

char** parse(const char*);

typedef struct ProcessNode
{
	//value? or void (*proc)()?
	ProcessNode* next;
} procNode;

typedef struct StakNode
{
	int procSize = 0;
	procNode* procList;
	//StakNode *prev?
	StakNode* next;
} stakNode;

void enqueue()
{

}

void dequeue()
{

}

int main(int argc, char* argv[])
{
	//stakNode* head = new stakNode;//staktop이 필요한데...
	//stack top이란?..
	//shell과 monitor프로세스 추가해야함.

	char** p = parse(" test ads ;  &12  34 ");

	return 0;
}


char** parse(const char* command)
{
	char* cmdPointer = (char*)command;
	vector<string> split;

	const int regSize = 3;
	regex reg[regSize] = { regex("(^\\s+)"), regex("^[^\\w\\d\\s]"), regex("^\\w+") };

	while (*cmdPointer != '\0')
	{
		int i;
		cmatch match;
		for (i = 0; i < regSize; i++)
		{
			if (regex_search((const char*)cmdPointer, (const char*)(cmdPointer + strlen(cmdPointer)), match, reg[i]))
				break;
		}

		switch (i)
		{
		case 0:
			break;
		default:
			split.push_back(match[0].str());
			break;
		}

		cmdPointer += match[0].str().length();
	}
	split.push_back("");

	char** result = (char**)malloc(sizeof(char*) * (split.size()));
	if (result == NULL)
		return NULL;

	for (unsigned int i = 0; i < split.size(); i++)
	{
		result[i] = _strdup((char*)split[i].c_str());
	}

	return result;
}