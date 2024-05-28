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
#include <map> 

using namespace std;


#define Y 3
#define DONE 200

enum class processList
{
	shell, monitor, echo, dummy, gcd, prime, sum
};

enum class processType
{
	Foreground, Background
};

class StakNode;
class ProcList;
struct ProcNode;

void enqueue(processList, processType);
void enqueue(processList);
void dequeue(StakNode*);
void promote();
void split_n_merge(StakNode*);
void shell();
void monitor();
void echo(string);
void gcd(string, string);
void prime(string);
void sum(string, string);
void sumTh(int, int, int*);
char** parse(const char*);
void exec(char**);
void make(vector<string>);

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
	//void Instert(ProcNode*)


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
	if (prev != nullptr)
	{
		this->next = prev->next;
		prev->next = this;
	}
	if (this->next != nullptr)
		this->next->prev = this;
	nodeSize++;
}

StakNode::~StakNode()
{
	if (prev != nullptr) prev->next = next;
	if (next != nullptr) next->prev = prev;
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
	int id;
	void (*func)() = nullptr;
	processType type = processType::Foreground;
	vector<string> args;
	int leftTime = DONE;
	int period = -1;
	int leftWait = 0;
	ProcNode* next = nullptr;

	ProcNode(int id)
	{
		this->id = id;
	}
};


ProcList::ProcList(StakNode* parent)
{
	this->parent = parent;
	this->start = nullptr;
	this->end = nullptr;
	_nodeCount = 0;

}

ProcList::~ProcList()
{
	_nodeCount--;
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
	if (end != nullptr)
		end->next = newNode;
	else
		start = newNode;
	end = newNode;

	_nodeCount++;
	procCount++;

	for (; end->next != nullptr; end = end->next)
	{
		_nodeCount++;
	}
}

ProcNode* ProcList::Remove()
{
	ProcNode* removed = start;
	start = start->next;
	if (start == nullptr)
		end = nullptr;
	_nodeCount--;
	procCount--;
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
		procCount--;
	}
	ProcNode* end = start;
	if (start != nullptr)
		start = start->next;
	else
		this->end = nullptr;
	if (end != nullptr)
		end->next = nullptr;

	return removed;
}
#pragma endregion ProcNode


mutex printMtx;

int id = 0;

StakNode* stakBottom = new StakNode(nullptr);
StakNode* stakTop = stakBottom;
StakNode* P = stakBottom;
ProcList WQ = ProcList(nullptr);

int main(int argc, char* argv[])
{
	//enqueue(shell, Foreground);
	//enqueue(monitor, Background);


	char** p = parse(" echo ads -n 2 ;  &12  34 ");
	exec(p);

	prime("97");

	sum("100", "3");

	return 0;
}

void enqueue(ProcNode* node)
{
	StakNode* addTo;

	if (node->type == processType::Foreground)
	{
		addTo = stakTop;
	}
	else
	{
		addTo = stakBottom;
	}

	addTo->procList()->Add(node);

	split_n_merge(addTo);
	promote();
}

void dequeue(StakNode* stak)
{
	ProcNode* deNode = stak->procList()->Remove();

	if(deNode->leftTime <= 0) delete deNode;

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
	StakNode* check = P;

	if ((P = P->NextNode()) == nullptr)
	{
		P = new StakNode(stakTop);
		stakTop = P;
	}

	P->procList()->Add(pNode);

	if (check->procList()->nodeCount() == 0)
	{
		if (check == stakBottom)
			stakBottom = stakBottom->NextNode();

		delete check;
	}

	if ((P = P->NextNode()) == nullptr)
		P = stakBottom;

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

void shell()
{
	while (true)
	{

	}
}

void monitor()
{
	while (true)
	{
		printMtx.lock();

		printMtx.unlock();
	}
}

void echo(string output)
{
	printMtx.lock();
	cout << output << endl;
	printMtx.unlock();
}

void gcd(string x, string y)
{
	int a = stoi(x), b = stoi(y), result;
	if (a < b)
	{
		result = a;
		a = b;
		b = result;
	}

	while (true)
	{
		int r = a % b;
		if (r == 0) {
			result = b;
			break;
		}
		a = b;
		b = r;
	}


	printMtx.lock();
	cout << result << endl;
	printMtx.unlock();
}

void prime(string x)
{
	int _x = stoi(x);

	if (_x < 2)
	{
		printMtx.lock();
		cout << 0 << endl;
		printMtx.unlock();
		return;
	}

	vector<int> primes = {2};

	for (int i = 3; i <= _x; i++)
	{
		bool check = true;
		for (int j = 0; j < primes.size(); j++)
		{
			if (i % primes[j] == 0)
			{
				check = false;
				break;
			}
		}

		if (check)
			primes.push_back(i);
	}

	printMtx.lock();
	cout << primes.size() << endl;
	printMtx.unlock();
}

mutex sumMtx;

void sum(string x, string m)
{
	int _x = stoi(x), _m = stoi(m);

	vector<thread*> sumThread;
	int result = _x;

	for (int i = 0; i < _m; i++)
	{
		int end = (i + 1 == _m) ? _x : (_x / _m) * (i + 1);
		sumThread.push_back(new thread(sumTh, (_x / _m) * i, end, &result));
	}

	for (int i = 0; i < sumThread.size(); i++)
	{
		sumThread[i]->join();
	}

	printMtx.lock();
	cout << result << endl;
	printMtx.unlock();
}

void sumTh(int start, int end, int* result)
{
	int value = 0;

	for (int i = start; i < end; i++)
	{
		value += i;
	}

	sumMtx.lock();
	*result += value;
	sumMtx.unlock();
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

void exec(char** args)
{
	char** reader = args; 
	vector<string> command;
	while (strcmp(*reader, "") != 0)
	{
		if (strcmp(*reader, ";") == 0)
		{
			make(command); 
			command.clear();
		}
		else
		{
			command.push_back(string(*reader));
		}

		free(*reader);
		reader++;
	}

	if (command.size())
	{
		make(command);
	}

	free(*reader);

	free(args);
}

void make(vector<string> args)
{
	processType type = processType::Foreground;
	if (args[0].compare("&") == 0)
		type = processType::Background;

	map<string, processList> procMap = { 
		{"echo", processList::echo},
		{"dummy", processList::dummy},
		{"gcd", processList::gcd},
		{"prime", processList::prime},
		{"sum", processList::sum}
	};
	auto find = procMap.find(args[type == processType::Foreground ? 0 : 1 ]);

	if (find == procMap.end())
		return;

	processList proc =	find->second;

	int n = 1, d = DONE, p = -1, m = 1;

	map<string, int*> optionMap = {
		{"n", &n},
		{"d", &d},
		{"p", &p},
		{"m", &m}
	};
	int* modif = nullptr;
	bool doModif = false;
	vector<string> _args;

	for (int i = (type == processType::Foreground ? 1 : 2); i < args.size(); i++)
	{
		if (args[i].compare("-") == 0)
		{
			doModif = true;
		}
		else if (modif != nullptr)
		{
			*modif = stoi(args[i]);
			modif = nullptr;
		}
		else if (doModif)
		{
			auto option = optionMap.find(args[i]);

			if (option == optionMap.end())
				return;

			modif = option->second;
		}
		else
			_args.push_back(args[i]);
	}
	if (proc == processList::sum)
		_args.push_back(to_string(m));

	for (int i = 0; i < n; i++)
	{
		ProcNode* newProc = new ProcNode(id++);

		newProc->type = type;

		switch (proc)
		{
		case processList::echo:
			newProc->func = (void(*)())echo;
			break;
		case processList::dummy:
			newProc->func = nullptr;
			break;
		case processList::gcd:
			newProc->func = (void(*)())gcd;
			break;
		case processList::prime:
			newProc->func = (void(*)())prime;
			break;
		case processList::sum:
			newProc->func = (void(*)())sum;
			break;
		default:
			break;
		}
		newProc->leftTime = d;
		newProc->period = p;

		newProc->args = _args;

		enqueue(newProc);
	}

}