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

class StakNode;
class ProcList;


#pragma region StakNode
class StakNode
{
public:
	StakNode();
	//procNode를 받는 오버로딩 구현하기
	~StakNode();
	int Count();

	ProcList* procList;//readonly가 없는가?..수정하지 못하게 _procList로 하고 procList함수로구현해볼까
	//캡슐화를 위해 밑에 변수들을 프라이빗으로 바꾸고 get만들어야한다.
	StakNode* prev = nullptr;
	StakNode* next = nullptr;
	//필요한 함수는? Add Remove
private:
	int procSize;
	static int nodeSize;
};

int StakNode::nodeSize = 0;

int StakNode::Count()
{
	return nodeSize;
}

StakNode::~StakNode()
{
	nodeSize--;
}

#pragma endregion StakNode

#pragma region ProcNode

class ProcList
{
public:
	struct ProcNode;
	ProcList(StakNode*);
	ProcList(StakNode*, ProcNode*);
	~ProcList();
	//캡슐화 하기
	//필요한 함수는? Add Remove split. split은 일정 크기 자르고 주소값 리턴하자. 이를 위해 ProcNode를 받는 생성자를 추가해야할듯
	

private:
	//쓰레드를 가지고 있어야할듯 아마 thread proc;
	StakNode* parent;
	ProcNode* start;
	ProcNode* end;
};

struct ProcList::ProcNode
{
	ProcNode* next = nullptr;
};

ProcList::ProcList(StakNode* parent)
{
	this->parent = parent;
	this->start = nullptr;
	this->end = nullptr;
}

ProcList::ProcList(StakNode* parent, ProcNode* newNode)
{
	this->parent = parent;
	start = newNode;
	for (end = start; end->next != nullptr; end = end->next);
}

ProcList::~ProcList()
{
}
	

#pragma endregion ProcNode

StakNode::StakNode()//선언 문제로 여기다가..
{
	procList = new ProcList(this); 
	this->procSize = 0;
	nodeSize++;
}


enum processType
{
	Foreground, Background
};

StakNode* stakBottom = new StakNode();
StakNode* stakTop = stakBottom;

int main(int argc, char* argv[])
{
	//stack top이란?..
	//shell(fore)과 monitor(back)프로세스 추가해야함.

	char** p = parse(" test ads ;  &12  34 ");

	return 0;
}

void enqueue()
{
	//if 프로세스 == foregroun? stakTop에 추가 : Bottom에 추가.


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
	//P가 따로 존재.
	//P가 stakTop일경우 다음 스택노드는 bottom으로.
	//꼬리에 붙여라. 인자넣어도 되겠지 뭐
}

void split_n_merge()
{
	//enqueue() 또는 promote() 수행시 실행됨
	//threshold = 전체프로세스개수/ 스택노드수. stak.ProcCount() / StakNode.Count()
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