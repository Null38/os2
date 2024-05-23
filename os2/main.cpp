/*
2-1
2-2
2-3
*/

#include <iostream>
#include <thread>
#include <mutex>
#include <windows.h>
#include <string>
#include <regex>
#include <vector>

using namespace std;


enum class processList
{
	shell, monitor
};

enum class processType
{
	Foreground, Background
};

class StakNode;
class ProcList;
struct ProcNode;

//이 함수들 void인거 나중에 바꿀 수 있다는점 인지하기.
void enqueue(processList*, processType);
void enqueue(processList*);
void dequeue(StakNode*);
void promote();
void split_n_merge(StakNode*);
char** parse(const char*);

class StakNode
{
public:
	StakNode();
	StakNode(struct ProcNode*);
	~StakNode();

	static int Count();
	ProcList* procList();
	//캡슐화를 위해 밑에 변수들을 프라이빗으로 바꾸고 get만들어야한다.
	//필요한 함수는? Add Remove
private:
	static int nodeSize;

	ProcList* _procList;

	StakNode* prev = nullptr;
	StakNode* next = nullptr;
};
int StakNode::nodeSize = 0;

class ProcList
{
public:
	ProcList(StakNode*);
	ProcList(StakNode*, ProcNode*);
	~ProcList();

	static int ProcCount();
	int nodeCount();

	//캡슐화 하기
	//필요한 함수는? Add Remove split. split은 일정 크기 자르고 주소값 리턴하자. 이를 위해 ProcNode를 받는 생성자를 추가해야할듯


private:
	static int procCount;
	int _nodeCount;
	StakNode* parent;
	ProcNode* start;
	ProcNode* end;
};
int ProcList::procCount = 0;

#pragma region StakNode

StakNode::StakNode()
{
	_procList = new ProcList(this);
	nodeSize++;
}

StakNode::StakNode(struct ProcNode* newStart)
{
	_procList = new ProcList(this, newStart);
	nodeSize++;
}


int StakNode::Count()
{
	return nodeSize;
}


ProcList* StakNode::procList()
{
	return _procList;
}

StakNode::~StakNode()
{
	nodeSize--;
	delete _procList;
}

#pragma endregion StakNode

#pragma region ProcNode
struct ProcNode
{
	//쓰레드를 가지고 있어야할듯 아마 thread proc;
	ProcNode* next = nullptr;
};

ProcList::ProcList(StakNode* parent)
{
	this->parent = parent;
	this->start = nullptr;
	this->end = nullptr;
	_nodeCount = 0;

}

ProcList::ProcList(StakNode* parent, ProcNode* newNode)
{
	this->parent = parent;
	start = newNode;
	for (end = start; end->next != nullptr; end = end->next)
	{
		_nodeCount++;
	}
	_nodeCount++;

}

ProcList::~ProcList()
{
}
	
int ProcList::ProcCount()
{
	return procCount;
}

int ProcList::nodeCount()
{
	return _nodeCount;
}

#pragma endregion ProcNode




StakNode* stakBottom = new StakNode();
StakNode* stakTop = stakBottom;
StakNode* P = stakBottom;

int main(int argc, char* argv[])
{
	//enqueue(shell, Foreground);
	//enqueue(monitor, Background);

	char** p = parse(" test ads ;  &12  34 ");

	return 0;
}

void enqueue(processList* proc, processType type)
{
	StakNode addTo;

	if (type == processType::Foreground)
	{
		addTo = *stakTop;
	}
	else
	{
		addTo = *stakBottom;
	}

	//addTo.procList().Add(proc);

	promote();
}

void enqueue(processList* proc)
{
	enqueue(proc, processType::Foreground);
	//split_n_merge(proc추가한 Node);
}

void dequeue(StakNode* stak)
{
	//if(procIsEnd) proc.RemoveProc(); -> in RemoveProc() if (procList == nullptr) Remove()

	promote();
}

void promote()
{
	//P가 따로 존재.
	//P가 stakTop일경우 다음 스택노드는 bottom으로.
	//꼬리에 붙여라. 인자넣어도 되겠지 뭐

	//split_n_merge(proc추가한 Node);
}

void split_n_merge(StakNode* stak)
{
	int threshold = ProcList::ProcCount() / StakNode::Count();
	if(stak->procList()->nodeCount() <= threshold)
		return;
	//절반을 자른다.. RemoveProc()에 몇개를 자를지 설정할 수 있는 오버로딩을 추가하는게 좋을듯. 구현한다면...
	//만약 my == StakTop? stak.Add();

	//split_n_merge(proc추가한 Node);
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