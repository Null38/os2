/*
2-1    
	1) O
	2) 
	3) O
	4) O
	5) 

2-2    
	1) 
	2) 
	3) O
	4) O

2-3    O
	1) O
	2) O
	3) O
*/

#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <windows.h>
#include <string>
#include <regex>
#include <vector>
#include <map> 

using namespace std;

#define X 5
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

void scheduler();
void enqueue(ProcNode*);
void dequeue(StakNode*);
void promote();
void split_n_merge(StakNode*);
void shell(ProcNode*);
void monitor(ProcNode*);
void echo(ProcNode*);
void gcd(ProcNode*);
void prime(ProcNode*);
void sum(ProcNode*);
void sumTh(int, int, int*);
char** parse(const char*);
void exec(char**);
void make(vector<string>);
void makeTh(ProcNode*);


#pragma region LinkedList
template<typename T>
class LinkedList 
{
public:
	struct Node;
	LinkedList();
	T* GetStart();
	T* GetEnd();
	void Add(T* data);
	void Add(Node* node);
	int NodeCount() { return nodeCount; }
	T* Remove();
	Node* Remove(int index);
	void insert(int(*)(T*), T*);
private:
	Node* start;
	Node* end;
	int nodeCount = 0;
};

template<typename T>
struct LinkedList<T>::Node
{
	T* data;

	Node(T* data)
	{
		this->data = data;
		nodeCount++;
	}

	~Node()
	{
		nodeCount--;
	}

private:
	Node* next = nullptr;
};

template<typename T>
LinkedList<T>::LinkedList()
{
	start = nullptr;
	end = nullptr;
}

template<typename T>
T* LinkedList<T>::GetStart()
{
	return start->data;
}

template<typename T>
T* LinkedList<T>::GetEnd()
{
	return end->data;
}

template<typename T>
void LinkedList<T>::Add(T* data)
{
	Node* newNode = new Node(data);
	if (start == nullptr)
		start = newNode;
	else
		end->next = newNode;

	end = newNode;
}

template<typename T>
void LinkedList<T>::Add(Node* node) 
{
	if (!start)
		start = node;
	else
		end->next = node;

	for (Node* temp = node; temp->next != nullptr; temp = temp->next)
	{
		nodeCount++;
	}

	end = node;
}

template<typename T>
T* LinkedList<T>::Remove() 
{
	if (start == nullptr) 
		return nullptr; 

	Node* temp = start;
	start = start->next; 

	T* data = temp->data; 
	delete temp; 

	nodeCount--;

	return data; 
}

template<typename T>
typename LinkedList<T>::Node* LinkedList<T>::Remove(int size)
{
	if (size <= 0 || start == nullptr)
		return nullptr;

	Node* data = start;
	Node* dataEnd = data;
	nodeCount--;
	for (int i = 1; i < size; i++)
	{
		if (dataEnd->next == nullptr)
			break;

		dataEnd = dataEnd->next;
		nodeCount--;
	}

	start = dataEnd->next;
	dataEnd->next = nullptr;

	if (start == nullptr)
		end = nullptr;
	
	return data;
}

template<typename T>
void LinkedList<T>::insert(int (*compareFunc)(T*), T* data)
{
	Node* newNode = new Node(data);

	if (start == nullptr)
	{
		start = newNode;
		end = newNode;
		nodeCount++;
		return;
	}

	Node* curr = start;
	Node* prev = nullptr;

	while (curr != nullptr && compareFunc(newNode) >= compareFunc(curr->data))
	{
		prev = curr;
		curr = curr->next;
	}

	if (prev == nullptr)
	{
		newNode->next = start;
		start = newNode;

		return;
	}

	prev->next = newNode;
	newNode->next = curr;
	if (curr == nullptr)
		end = newNode;
}
#pragma endregion LinkedList


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
	void Insert(ProcNode*);
	ProcNode* GetStartInfo();


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
	bool isPromoted = false;
	int leftTime = DONE;
	int startAt;
	int period = -1;
	int leftWait = 0;
	ProcNode* next = nullptr;

	ProcNode(int id, int startSec)
	{
		this->id = id;
		startAt = startSec;
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
		procCount++;
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
	_nodeCount--;
	procCount--;
	return removed;
}


void  ProcList::Insert(ProcNode* newNode)
{
	/*newNode->leftWait = newNode->period;

	ProcNode* i;
	for (i = start; i->next != nullptr; i = i->next)
	{
		if (i->leftWait >= newNode->leftWait)
			break;
	}

	i->next = newNode->next;
	newNode->next = i;

	_nodeCount++;
	procCount++;*/
}

ProcNode* ProcList::GetStartInfo()
{
	return start;
}
#pragma endregion ProcNode


mutex printMtx;

int id = 0;
int sec = 0;

StakNode* stakBottom = new StakNode(nullptr);
StakNode* stakTop = stakBottom;
StakNode* P = stakBottom;
ProcList WQ = ProcList(nullptr);
ProcNode* running;

ifstream command;

int main(int argc, char* argv[])
{
	command.open("command.txt");

	ProcNode* instProc = new ProcNode(id++, sec);
	instProc->func = (void(*)())shell;
	instProc->type = processType::Foreground;
	instProc->period = Y;
	enqueue(instProc);

	instProc = new ProcNode(id++, sec);
	instProc->func = (void(*)())monitor;
	instProc->type = processType::Background;
	instProc->period = X;
	enqueue(instProc);

	while (true)
	{
		scheduler();
	}

	command.close();
	return 0;
}

void scheduler()
{
	sec++;
	dequeue(stakTop);
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
	else WQ.Insert(deNode);

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
	bool isNew = false;
	if ((P = P->NextNode()) == nullptr)
	{
		P = new StakNode(stakTop);
		stakTop = P;
		isNew = true;
	}

	P->procList()->Add(pNode);
	pNode->isPromoted = true;

	if (check->procList()->nodeCount() == 0)
	{
		if (check == stakBottom)
			stakBottom = stakBottom->NextNode();

		delete check;
	}

	if (isNew)
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
	
	StakNode* check = stak;

	if ((stak = stak->NextNode()) == nullptr)
	{
		stakTop = new StakNode(stakTop);
		stak = stakTop;
	}


	if (check->procList()->ProcCount() == 0)
	{
		if (check == stakBottom)
			stakBottom = stakBottom->NextNode();

		delete check;
	}

	stak->procList()->Add(moveNode);
	
	split_n_merge(stak);
}

void shell(ProcNode* proc)
{
	string line;

	getline(command, line);

	printMtx.lock();
	cout << "prompt>" << line << endl;
	printMtx.unlock();

	exec(parse(line.c_str()));
}

void monitor(ProcNode* proc)
{
	printMtx.lock();
	cout << "Running: ";
	
	cout << endl << "---------------------------" << endl;

	cout << "DQ: ";
	
	for (StakNode* stak = stakBottom; stak!= nullptr; stak = stak->NextNode())
	{
		cout << (P == stak ? "P => [" : "     [" );

		for (ProcNode* proc = stak->procList()->GetStartInfo(); proc != nullptr; proc = proc->next)
		{
			cout << (proc->isPromoted ? "*" : "")
				<< proc->id 
				<< (proc->type == processType::Foreground ? "F" : "B")
				<< (proc->next != nullptr ? " " : "");

			proc->isPromoted = false;
		}
		cout << "]";

		bool isBottom = false, isTop = false;
		if (stak == stakBottom)
			isBottom = true;
		if (stak == stakTop)
			isTop = true;

		cout << (isBottom || isTop ? "(" : "")
			<< (isBottom ? "bottom" : "")
			<< (isBottom && isTop ? "/" : "")
			<< (isTop ? "top" : "")
			<< (isBottom || isTop ? ")" : "");

		if (stak != stakTop)
			cout << endl << "    " ;
	}

	cout << endl << "---------------------------" << endl;
	cout << "WQ: [";
	for (ProcNode* proc = WQ.GetStartInfo(); proc != nullptr; proc = proc->next)
	{
		cout << proc->id
			<< (proc->type == processType::Foreground ? "F" : "B")
			<< (proc->next != nullptr ? " " : "");
		//  << leftTime
	}
	cout << "]" << endl;
	printMtx.unlock();
}

void echo(ProcNode* proc) 
{
	printMtx.lock();
	cout << proc->args[0] << endl;
	printMtx.unlock();
}

void gcd(ProcNode* proc) 
{
	int a = stoi(proc->args[0]), b = stoi(proc->args[1]), result;
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

void prime(ProcNode* proc) 
{
	int _x = stoi(proc->args[0]);

	if (_x < 2)
	{
		printMtx.lock();
		cout << 0 << endl;
		printMtx.unlock();
		return;
	}

	vector<int> primes = { 2 };

	for (int i = 3; i <= _x; i++) 
	{
		bool check = true;
		for (size_t j = 0; j < primes.size(); j++) 
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

void sum(ProcNode* proc) 
{
	int _x = stoi(proc->args[0]), _m = stoi(proc->args[1]);

	vector<thread*> sumThread;
	int result = _x;

	for (int i = 0; i < _m; i++) 
	{
		int end = (i + 1 == _m) ? _x : (_x / _m) * (i + 1);
		sumThread.push_back(new thread(sumTh, (_x / _m) * i, end, &result));
	}

	for (size_t i = 0; i < sumThread.size(); i++) 
	{
		sumThread[i]->join();
	}

	printMtx.lock();
	cout << result % 100000 << endl;
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

	for (size_t i = (type == processType::Foreground ? 1 : 2); i < args.size(); i++)
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
		ProcNode* newProc = new ProcNode(id++, sec);

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
		makeTh(newProc);
	}

}

void makeTh(ProcNode* proc)
{
	//string arg0(proc->args.size() >= 1 ? proc->args[0] : ""), arg1(proc->args.size() >= 2 ? proc->args[1] : "");
	//thread th((void(*)(string, string))proc->func, arg0, arg1);
	//if proc.type == FG
	//	joln?
	//else
	//	detatch?
}