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

template<typename T>
class LinkedList;
class StackNode;
struct ProcNode;

void init();
void scheduler();
void enqueue(ProcNode*);
void dequeue(StackNode*);
void promote();
void split_n_merge(StackNode*);
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
	Node* GetStart();
	Node* GetEnd();
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


#pragma region Node
template<typename T>
struct LinkedList<T>::Node
{
	LinkedList<T>* parent;

	T* data;

	Node(LinkedList<T>*, T*);
	~Node();

	Node* NextNode();
private:
	Node* next = nullptr;
};

template<typename T>
LinkedList<T>::Node::Node(LinkedList<T>* parent, T* data)
{
	this->parent = parent;
	this->data = data;
	nodeCount++;
}

template<typename T>
LinkedList<T>::Node::~Node()
{
	nodeCount--;
}

template<typename T>
LinkedList<T>::Node* LinkedList<T>::Node::NextNode()
{
	return nullptr;
}
#pragma endregion Node

template<typename T>
LinkedList<T>::LinkedList()
{
	start = nullptr;
	end = nullptr;
}

template<typename T>
LinkedList<T>::Node* LinkedList<T>::GetStart()
{
	return start->data;
}

template<typename T>
LinkedList<T>::Node* LinkedList<T>::GetEnd()
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

#pragma region stackNode
struct StackNode
{
public:
	StackNode();
	~StackNode();

	static int Count();
	LinkedList<ProcNode> procList();

private:
	static int nodeSize;

	LinkedList<ProcNode> _procList;
};
int StackNode::nodeSize = 0;

StackNode::StackNode()
{
	nodeSize++;
}

StackNode::~StackNode()
{
	nodeSize--;
}

int StackNode::Count()
{
	return nodeSize;
}

#pragma endregion stackNode

#pragma region ProcNode
struct ProcNode
{
	static int procCount;

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

int ProcNode::procCount = 0;
#pragma endregion ProcNode


mutex printMtx;

int id = 0;
int sec = 0;

LinkedList<StackNode> stackList;
LinkedList<StackNode>::Node* P;
LinkedList<ProcNode> WQ;
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

void init()
{
	stackList.Add(new StackNode());
}

void scheduler()
{
	sec++;
	dequeue(stackList.GetEnd()->data);
}

void enqueue(ProcNode* node)
{
	StackNode* addTo;

	if (node->type == processType::Foreground)
	{
		addTo = stackList.GetEnd()->data;
	}
	else
	{
		addTo = stackList.GetStart()->data;
	}

	addTo->procList().Add(node);

	split_n_merge(addTo);
	promote();
}

void dequeue(StackNode* stack)
{
	ProcNode* deNode = stack->procList().Remove();

	if (deNode->leftTime <= 0) delete deNode;
	//else WQ.Insert(/* ÇÔ¼ö*/,deNode);

	/*
	if (stack->procList()->nodeCount() == 0)
	remove stack
	*/

	promote();
}

void promote()
{
	ProcNode* pNode = P->data->procList().Remove();
	LinkedList<StackNode>::Node* check = P;
	bool isNew = false;
	if ((P = P->NextNode()) == nullptr)
	{
		P->parent->Add(new StackNode());
		P = stackList.GetEnd();
		isNew = true;
	}

	P->data->procList().Add(pNode);
	pNode->isPromoted = true;

	/*
	if (check->procList()->nodeCount() == 0)
		remove check
	*/

	if (isNew)
		P = stackList.GetStart();

	split_n_merge(P->data);
}

void split_n_merge(LinkedList<StackNode>::Node* stack)
{
	int threshold = ProcNode::procCount / StackNode::Count();
	int count;
	if ((count = stack->data->procList().NodeCount()) <= threshold)
		return;

	LinkedList<ProcNode>::Node* moveNode = stack->data->procList().Remove(count / 2);

	LinkedList<StackNode>::Node* check = stack;

	if ((stack = stack->NextNode()) == nullptr)
	{
		stackList.Add(new StackNode());
	}

	/*
	if (check->data->procList().NodeCount() == 0)
	remove check
	*/

	stack->data->procList().Add(moveNode);

	split_n_merge(stack);
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

	for (LinkedList<StackNode>::Node* stack = stackList.GetStart(); stack != nullptr; stack = stack->NextNode())
	{
		cout << (P == stack ? "P => [" : "     [");

		for (ProcNode* proc = stack->data->procList().GetStart()->data; proc != nullptr; proc = proc->next)
		{
			cout << (proc->isPromoted ? "*" : "")
				<< proc->id
				<< (proc->type == processType::Foreground ? "F" : "B")
				<< (proc->next != nullptr ? " " : "");

			proc->isPromoted = false;
		}
		cout << "]";

		bool isBottom = false, isTop = false;
		if (stack == stackList.GetStart())
			isBottom = true;
		if (stack == stackList.GetEnd())
			isTop = true;

		cout << (isBottom || isTop ? "(" : "")
			<< (isBottom ? "bottom" : "")
			<< (isBottom && isTop ? "/" : "")
			<< (isTop ? "top" : "")
			<< (isBottom || isTop ? ")" : "");

		if (stack != stackList.GetEnd())
			cout << endl << "    ";
	}

	cout << endl << "---------------------------" << endl;
	cout << "WQ: [";
	for (ProcNode* proc = WQ.GetStart()->data; proc != nullptr; proc = proc->next)
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
	regex reg[regSize] = { regex("(^\\s+)"), regex("^[^\\w\\d\\s]"), regex("^\\w+") , regex("^.") };

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
	auto find = procMap.find(args[type == processType::Foreground ? 0 : 1]);

	if (find == procMap.end())
		return;

	processList proc = find->second;

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