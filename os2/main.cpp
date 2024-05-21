#include <iostream>
#include <thread>
#include <mutex>
#include <windows.h>
#include <string>
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

	char** p = parse(" test   12  34 ");

	return 0;
}


char** parse(const char* command)
{
	vector<string> split;

	int startAt = 0;
	for (unsigned int i = 0; command[i] != '\0'; i++)
	{
		if (command[startAt] == ' ')
		{
			startAt++;
			continue;
		}
		if (command[i] != ' ')
			continue;

		split.push_back(((string)command).substr(startAt, i - startAt));
		startAt = i + 1;
	}

	if (startAt < ((string)command).length())
	{
		split.push_back(((string)command).substr(startAt));
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