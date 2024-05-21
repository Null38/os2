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

enum processType
{
	Foreground, Background
};

typedef struct ProcessNode
{
	//�����带 ������ �־���ҵ� �Ƹ� thread proc;
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
	//stack top�̶�?..
	//shell(fore)�� monitor(back)���μ��� �߰��ؾ���.

	char** p = parse(" test ads ;  &12  34 ");

	return 0;
}

void enqueue()
{
	procNode* proc = NULL/* �����ؾ� �� �κ�. ���μ����� foregroun�ΰ�? */ == Foreground ? stakTop->procList : stakBottom->procList;

	for (; proc->next != nullptr; proc = proc->next);

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
	//���� stakTop�ϰ�� ���� ���ó���? ������ nullptr�̴�. ������ ���� top�ΰ�쿣 �ٽ� bottom���� ���°ǰ�?
	//������ �ٿ���. ���ڸ� ���� �� �ִٸ� ������ �ϴ� �����.
}

void split_n_merge()
{
	//enqueue() �Ǵ� promote() ����� �����
	//threshold = ��ü���μ�������/ ���ó���.. stak�� proc����Ʈ�� class�� ���� static������ �صδ°� ������. �׷��� �����ϴ°� ���� �� ��.
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