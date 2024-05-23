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
void enqueue(processList, processType);
void enqueue(processList);
void dequeue(StakNode*);
void promote();
void split_n_merge(StakNode*);
char** parse(const char*);

class StakNode
{
public:
	StakNode(StakNode*);
	~StakNode();

	static int Count();
	ProcList* procList();

	StakNode* NextNode();

private:
	static int nodeSize;

	ProcList* _procList;

	StakNode* prev;
	StakNode* next = nullptr;
};
int StakNode::nodeSize = 0;

class ProcList
{
public:
	ProcList(StakNode*);
	~ProcList();

	static int ProcCount();
	int nodeCount();

	void Add(ProcNode*);
	ProcNode* Remove();
	ProcNode* Remove(int);


private:
	static int procCount;
	int _nodeCount;
	StakNode* parent;
	ProcNode* start;
	ProcNode* end;
};
int ProcList::procCount = 0;

#pragma region StakNode

StakNode::StakNode(StakNode* prev)
{
	_procList = new ProcList(this);
	this->prev = prev;
	if(prev != nullptr) prev->next = this;
	nodeSize++;
}

StakNode::~StakNode()
{
	if (prev != nullptr) prev->next = next;
	next->prev = prev;
	nodeSize--;
	
	delete _procList;
}

int StakNode::Count()
{
	return nodeSize;
}


ProcList* StakNode::procList()
{
	return _procList;
}

StakNode* StakNode::NextNode()
{
	return next;
}

#pragma endregion StakNode

#pragma region ProcNode
struct ProcNode
{
	//쓰레드를 가지고 있어야할듯 아마 thread proc;
	processType type = processType::Foreground;
	ProcNode* next = nullptr;
};

ProcList::ProcList(StakNode* parent)
{
	this->parent = parent;
	this->start = nullptr;
	this->end = nullptr;
	_nodeCount = 0;

}

//ProcList::ProcList(StakNode* parent, ProcNode* newNode)
//{
//	this->parent = parent;
//	start = newNode;
//
//	_nodeCount = 0;
//	for (end = start; end->next != nullptr; end = end->next)
//	{
//		_nodeCount++;
//	}
//	_nodeCount++;
//
//}

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

void ProcList::Add(ProcNode* newNode)
{
	end->next = newNode;
	_nodeCount++;

	for (; end->next != nullptr; end = end->next)
	{
		_nodeCount++;
	}
}

ProcNode* ProcList::Remove()
{
	ProcNode* removed = start;
	start = start->next;
	_nodeCount--;
	removed->next = nullptr;

	return removed;
}


ProcNode* ProcList::Remove(int size)
{
	ProcNode* removed = start;
	for (int i = 0; i < size - 1; i++)
	{
		start = start->next;
		_nodeCount--;
	}
	ProcNode* end = start;
	start = start->next;
	end->next = nullptr;

	return removed;
}
#pragma endregion ProcNode




StakNode* stakBottom = new StakNode(nullptr);
StakNode* stakTop = stakBottom;
StakNode* P = stakBottom;

int main(int argc, char* argv[])
{
	//enqueue(shell, Foreground);
	//enqueue(monitor, Background);

	char** p = parse(" test ads ;  &12  34 ");

	return 0;
}

void enqueue(processList proc, processType type)
{
	StakNode* addTo;

	if (type == processType::Foreground)
	{
		addTo = stakTop;
	}
	else
	{
		addTo = stakBottom;
	}

	//addTo->procList()->Add(proc);

	promote();
	split_n_merge(addTo);
}

void enqueue(processList proc)
{
	enqueue(proc, processType::Foreground);
}

void dequeue(StakNode* stak)
{
	ProcNode* deNode = stak->procList()->Remove();

	//if(procIsEnd) delete deNode;

	if (stak->procList()->nodeCount() == 0)
	{
		if (stak == stakBottom)
			stakBottom = stakBottom->NextNode();

		delete stak;
	}

	promote();
}

void promote()
{
	ProcNode* pNode =  P->procList()->Remove();

	if ((P = P->NextNode()) == nullptr)
		P = stakBottom;
	P->procList()->Add(pNode);

	split_n_merge(P);
}

void split_n_merge(StakNode* stak)
{
	int threshold = ProcList::ProcCount() / StakNode::Count();
	int count;
	if((count = stak->procList()->nodeCount()) <= threshold)
		return;

	ProcNode* moveNode = stak->procList()->Remove(count / 2);
	
	if ((stak = stak->NextNode()) == nullptr)
	{
		stakTop = new StakNode(stakTop);
		stak = stakTop;
	}

	stak->procList()->Add(moveNode);

	split_n_merge(stak);
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