#include <iostream>
#include <thread>
#include <mutex>
#include <windows.h>
#include <string>
#include <regex>
#include <vector>

using namespace std;

void enqueue();//�̰� void�ΰ� ���߿� �ٲ� �� �ִٴ��� �����ϱ�.
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
	//procNode�� �޴� �����ε� �����ϱ�
	~StakNode();
	int Count();

	ProcList* procList;//readonly�� ���°�?..�������� ���ϰ� _procList�� �ϰ� procList�Լ��α����غ���
	//ĸ��ȭ�� ���� �ؿ� �������� �����̺����� �ٲٰ� get�������Ѵ�.
	StakNode* prev = nullptr;
	StakNode* next = nullptr;
	//�ʿ��� �Լ���? Add Remove
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
	//ĸ��ȭ �ϱ�
	//�ʿ��� �Լ���? Add Remove split. split�� ���� ũ�� �ڸ��� �ּҰ� ��������. �̸� ���� ProcNode�� �޴� �����ڸ� �߰��ؾ��ҵ�
	

private:
	//�����带 ������ �־���ҵ� �Ƹ� thread proc;
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

StakNode::StakNode()//���� ������ ����ٰ�..
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
	//stack top�̶�?..
	//shell(fore)�� monitor(back)���μ��� �߰��ؾ���.

	char** p = parse(" test ads ;  &12  34 ");

	return 0;
}

void enqueue()
{
	//if ���μ��� == foregroun? stakTop�� �߰� : Bottom�� �߰�.


	//Add(proc, /*process*/); // Ŭ������ �����ؼ� proc.Add(/*process*/)�� �����غ����? ���Ḯ��Ʈ�� 2���� �װ� �� ����Ҽ���?
}

void dequeue()
{
	//� ������ ���μ����� ����ġ�ϴ°�?.. ���ڸ� �޾ƾ��ϴ°�?
	//if(procIsEnd) proc.RemoveProc(); -> in RemoveProc() if (procList == nullptr) Remove()
}

void promote()
{
	//enqueue() �Ǵ�dequeue() ����� �����
	//P�� ���� ����.
	//P�� stakTop�ϰ�� ���� ���ó��� bottom����.
	//������ �ٿ���. ���ڳ־ �ǰ��� ��
}

void split_n_merge()
{
	//enqueue() �Ǵ� promote() ����� �����
	//threshold = ��ü���μ�������/ ���ó���. stak.ProcCount() / StakNode.Count()
	//if(my->procList.Count() <= threshold) return; ������ �ʿ� ���׶�� �Ǵ�.
	//������ �ڸ���.. RemoveProc()�� ��� �ڸ��� ������ �� �ִ� �����ε��� �߰��ϴ°� ������. �����Ѵٸ�...
	//���� my == StakTop? stak.Add();
	//�̰� �ϴ� �����ϰ� ��������� �ٽ� ȣ��. ��Ͱ� �������� ������ ������.
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