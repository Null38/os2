#include <iostream>
#include <thread>
#include <mutex>
#include <windows.h>
#include <string>
#include <regex>
#include <vector>

using namespace std;

void enqueue();//이거 void인거 나중에 바꿀 수 있다는점 인지하기.
void dequeue();
void promote();
void split_n_merge();
char** parse(const char*);

enum processType
{
	Foreground, Background
};

typedef struct ProcessNode
{
	//쓰레드를 가지고 있어야할듯 아마 thread proc;
	struct StakNode* parent;
	ProcessNode* next;
} procNode;

typedef struct StakNode
{
	int procSize = 0;
	procNode* procList = nullptr;
	StakNode* prev = nullptr;
	StakNode* next = nullptr;
} stakNode;

stakNode* stakBottom = new stakNode;
stakNode* stakTop = stakBottom;

int main(int argc, char* argv[])
{
	//stack top이란?..
	//shell(fore)과 monitor(back)프로세스 추가해야함.

	char** p = parse(" test ads ;  &12  34 ");

	return 0;
}

void enqueue()
{
	procNode* proc = NULL/* 수정해야 할 부분. 프로세스가 foregroun인가? */ == Foreground ? stakTop->procList : stakBottom->procList;

	for (; proc->next != nullptr; proc = proc->next);

	//Add(proc, /*process*/); // 클래스로 선언해서 proc.Add(/*process*/)로 변경해보기는? 연결리스트가 2개라 그게 더 깔끔할수도?
}

void dequeue()
{
	//어떤 스택의 프로세스를 디스패치하는것?.. 인자를 받아야하는가?
	//if(procIsEnd) proc.RemoveProc(); -> in RemoveProc() if (procList == nullptr) Remove()
}

void promote()
{
	//enqueue() 또는dequeue() 수행시 실행됨
	//내가 stakTop일경우 다음 스택노드는? 상위는 nullptr이다. 과제를 보면 top인경우엔 다시 bottom으로 가는건가?
	//꼬리에 붙여라. 인자를 받을 수 있다면 구현은 일단 쉬울듯.
}

void split_n_merge()
{
	//enqueue() 또는 promote() 수행시 실행됨
	//threshold = 전체프로세스개수/ 스택노드수.. stak과 proc리스트를 class로 한후 static변수로 해두는게 맞을듯. 그렇게 관리하는게 나을 듯 함.
	//if(my->procList.Count() <= threshold) return; 실행할 필요 없네라는 판단.
	//절반을 자른다.. RemoveProc()에 몇개를 자를지 설정할 수 있는 오버로딩을 추가하는게 좋을듯. 구현한다면...
	//만약 my == StakTop? stak.Add();
	//이걸 일단 수행하고 재귀적으로 다시 호출. 재귀가 끝날때는 위같은 경우겠지.
}

char** parse(const char* command)
{
	char* cmdPointer = (char*)command;
	vector<string> split;

	const int regSize = 4;
	regex reg[regSize] = { regex("(^\\s+)"), regex("^[^\\w\\d\\s]"), regex("^\\w+") , regex("^.")};

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