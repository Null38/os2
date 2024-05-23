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

//�� �Լ��� void�ΰ� ���߿� �ٲ� �� �ִٴ��� �����ϱ�.
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
	//ĸ��ȭ�� ���� �ؿ� �������� �����̺����� �ٲٰ� get�������Ѵ�.
	//�ʿ��� �Լ���? Add Remove
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

	//ĸ��ȭ �ϱ�
	//�ʿ��� �Լ���? Add Remove split. split�� ���� ũ�� �ڸ��� �ּҰ� ��������. �̸� ���� ProcNode�� �޴� �����ڸ� �߰��ؾ��ҵ�


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
	//�����带 ������ �־���ҵ� �Ƹ� thread proc;
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
	//split_n_merge(proc�߰��� Node);
}

void dequeue(StakNode* stak)
{
	//if(procIsEnd) proc.RemoveProc(); -> in RemoveProc() if (procList == nullptr) Remove()

	promote();
}

void promote()
{
	//P�� ���� ����.
	//P�� stakTop�ϰ�� ���� ���ó��� bottom����.
	//������ �ٿ���. ���ڳ־ �ǰ��� ��

	//split_n_merge(proc�߰��� Node);
}

void split_n_merge(StakNode* stak)
{
	int threshold = ProcList::ProcCount() / StakNode::Count();
	if(stak->procList()->nodeCount() <= threshold)
		return;
	//������ �ڸ���.. RemoveProc()�� ��� �ڸ��� ������ �� �ִ� �����ε��� �߰��ϴ°� ������. �����Ѵٸ�...
	//���� my == StakTop? stak.Add();

	//split_n_merge(proc�߰��� Node);
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