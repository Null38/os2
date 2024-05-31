/*
2-1    -
	1) O
	2) O
	3) O
	4) O
	5) X

2-2    -
	1) X
	2) O
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

#define X 50
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
struct StackNode;
struct ProcNode;

#pragma region LinkedList
template<typename T>
class LinkedList
{
public:
	struct Node;
	LinkedList();
	Node* GetBottom() { return bottom; }
	Node* GetTop() { return top; }
	int NodeCount() { return nodeCount; }

	void Add(T* data);
	void Add(Node* node);
	T* Remove();
	Node* Remove(int index);
	void Insert(int (*compareFunc)(T*), T*);
	void DeleteRequest(Node*);
private:
	Node* bottom = nullptr;
	Node* top = nullptr;
	int nodeCount = 0;
};


#pragma region Node
template<typename T>
struct LinkedList<T>::Node
{
	friend class LinkedList<T>;
	LinkedList<T>* parent;

	T* data;

	Node(LinkedList<T>*, T*);
	~Node();

	Node* NextNode() { return next; }
protected:
	Node* prev = nullptr;
	Node* next = nullptr;
};

template<typename T>
LinkedList<T>::Node::Node(LinkedList<T>* parent, T* data)
	: parent(parent), data(data)
{
	parent->nodeCount++;
}

template<typename T>
LinkedList<T>::Node::~Node()
{
	parent->nodeCount--;
}
#pragma endregion Node

template<typename T>
LinkedList<T>::LinkedList() {}

template<typename T>
void LinkedList<T>::Add(T* data)
{
	Node* newNode = new Node(this, data);
	if (!bottom)
		bottom = newNode;
	else
	{
		newNode->prev = top;
		top->next = newNode;
	}

	top = newNode;
}

template<typename T>
void LinkedList<T>::Add(Node* newNode)
{
	if (!newNode)
		return;

	if (!bottom)
		bottom = newNode;
	else
	{
		newNode->prev = top;
		top->next = newNode;
	}
	nodeCount++;
	newNode->parent = this;

	Node* temp;
	for (temp = newNode; temp->next; temp = temp->next)
	{
		nodeCount++;
		temp->parent = this;
	}

	top = temp;
}

template<typename T>
T* LinkedList<T>::Remove()
{
	if (!bottom)
		return nullptr;

	Node* temp = bottom;
	bottom = bottom->next;
	if (bottom)
		bottom->prev = nullptr;

	T* data = temp->data;
	delete temp;

	return data;
}

template<typename T>
typename LinkedList<T>::Node* LinkedList<T>::Remove(int size)
{
	if (size <= 0 || !bottom)
		return nullptr;

	Node* data = bottom;
	Node* temp = data;
	nodeCount--;
	temp->parent = nullptr;
	for (int i = 1; i < size && temp->next; i++)
	{
		nodeCount--;
		temp->parent = nullptr;
		temp = temp->next;
	}

	bottom = temp->next;
	if (bottom)
		bottom->prev = nullptr;

	data->prev = nullptr;
	temp->next = nullptr;

	if (!bottom)
		top = nullptr;

	return data;
}

template<typename T>
void LinkedList<T>::Insert(int (*compareFunc)(T*), T* data)
{
	Node* newNode = new Node(this, data);

	if (!bottom)
	{
		bottom = newNode;
		top = newNode;
		return;
	}

	Node* curr = bottom;
	Node* prev = nullptr;

	while (curr && compareFunc(newNode->data) >= compareFunc(curr->data))
	{
		prev = curr;
		curr = curr->next;
	}

	if (!prev)
	{
		newNode->next = bottom;
		bottom->prev = newNode;
		bottom = newNode;
		return;
	}

	prev->next = newNode;
	newNode->prev = prev;
	newNode->next = curr;
	if (!curr)
		top = newNode;
}

template<typename T>
void LinkedList<T>::DeleteRequest(Node* node)
{
	if (!node)
		return;

	if (!node->prev)
		bottom = node->next;
	else
		node->prev->next = node->next;

	if (!node->next)
		top = node->prev;
	else
		node->next->prev = node->prev;

	delete node;
}
#pragma endregion LinkedList

#pragma region StackNode
struct StackNode
{
public:
	StackNode();
	~StackNode();

	static int Count() { return nodeSize; }
	LinkedList<ProcNode>* procList() { return &_procList; }

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
#pragma endregion StackNode

#pragma region ProcNode
struct ProcNode
{
	struct ProcInfo
	{
		void (*func)(ProcInfo*) = nullptr;
		int id = 0;
		processType type = processType::Foreground;
		vector<string> args;
	};
	ProcInfo info;

	bool isPromoted = false;
	int lifeTime;
	int period;
	int leftWait = 0;

	ProcNode(int, processType, void (*func)(ProcInfo*), int, int, int);
	ProcNode(int id, void (*func)(ProcInfo*), int lifeTime, int period, int startSec)
		: ProcNode(id, processType::Foreground, func, lifeTime, period, startSec) {}
	ProcNode(int, processType, int, int, int);
	~ProcNode();

	static int Count() { return procCount; }
	static int GetLeftTime(ProcNode*);

private:
	static int procCount;
};
int ProcNode::procCount = 0;

ProcNode::ProcNode(int id, processType type, void (*func)(ProcInfo*), int lifeTime, int period, int startSec)
	: ProcNode(id, type, lifeTime, period, startSec)
{
	this->info.func = func;
}

ProcNode::ProcNode(int id, processType type, int lifeTime, int period, int startSec)
	: lifeTime(lifeTime + startSec), period(period)
{
	info.id   = id;
	info.type = type;
	procCount++;
}

ProcNode::~ProcNode()
{
	procCount--;
}

int ProcNode::GetLeftTime(ProcNode* proc)
{
	return proc->leftWait;
}
#pragma endregion ProcNode

#pragma region func_forward_declaration
void Init();
void scheduler();
void mtxEnqueue(ProcNode* node);
void mtxDequeue();
void enqueue(ProcNode*);
void dequeue();
void promote();
void split_n_merge(LinkedList<StackNode>::Node*);
void shell(ProcNode::ProcInfo*);
void monitor(ProcNode::ProcInfo*);
void echo(ProcNode::ProcInfo*);
void gcd(ProcNode::ProcInfo*);
void prime(ProcNode::ProcInfo*);
void sum(ProcNode::ProcInfo*);
void sumTh(int, int, int*);
char** parse(const char*);
void exec(char**);
void Make(vector<string>);
void MakeTh(ProcNode*);
void ProcessRunner(ProcNode::ProcInfo, thread*);
#pragma endregion func_forward_declaration

mutex printMtx;
mutex qeueuMtx;

int id = 0;
int sec = 0;

LinkedList<StackNode> stackList;
LinkedList<StackNode>::Node* P;
LinkedList<ProcNode> WQ;
ProcNode* running;
thread* fg = nullptr;

mutex fgMtx;

ifstream command;

int main(int argc, char* argv[])
{
	Init();
	command.open("command.txt");

	mtxEnqueue(new ProcNode(id++, shell, 1000, Y, sec));
	mtxEnqueue(new ProcNode(id++, processType::Background, monitor, 2147483647, X, sec));

	while (stackList.NodeCount())
	{
		scheduler();
	}

	command.close();


	if (fg != nullptr)
	{
		fgMtx.lock();
		fg->join();
		fgMtx.unlock();
		delete fg;
	}

	return 0;
}

void Init()
{
	stackList.Add(new StackNode());
	P = stackList.GetTop();
}

void scheduler()
{
	sec++;

	LinkedList<ProcNode>::Node* end = nullptr;
	for (auto temp = WQ.GetBottom(); temp; temp = temp->NextNode())
	{
		if (end)
		{
			auto data = end->data;
			if (temp->data->lifeTime - sec < 0)
			{
				end->parent->DeleteRequest(end);
				delete data;
			}
			else
			{
				end->parent->DeleteRequest(end);
				mtxEnqueue(data);
			}
			end = nullptr;
		}

		temp->data->leftWait--;

		if (temp->data->leftWait <= 0)
		{
			end = temp;
		}
	}

	if (stackList.GetBottom()->data->procList()->NodeCount() != 0)
	{
		mtxDequeue();
		if (stackList.NodeCount() == 0)
			return;

	}
	if (running == nullptr)
		return;

	MakeTh(running);

	if (running->period == -1)
		running->leftWait = running->lifeTime - sec;
	else
		running->leftWait = running->period;
	WQ.Insert(&ProcNode::GetLeftTime, running);
	running = nullptr;
}

#pragma region Dynamic_Queueing
void mtxEnqueue(ProcNode* node)
{
	qeueuMtx.lock();
	enqueue(node);
	qeueuMtx.unlock();
}

void mtxDequeue()
{
	qeueuMtx.lock();
	dequeue();
	qeueuMtx.unlock();
}

void enqueue(ProcNode* node)
{
	if (!node)
		return;

	LinkedList<StackNode>::Node* addTo;

	if (node->info.type == processType::Foreground)
	{
		addTo = stackList.GetTop();
	}
	else
	{
		addTo = stackList.GetBottom();
	}

	addTo->data->procList()->Add(node);

	split_n_merge(addTo);
	promote();
}

void dequeue()
{
	auto top = stackList.GetTop();

	ProcNode* deNode = top->data->procList()->Remove();
	if (deNode == nullptr)
		return;

	if (deNode->lifeTime - sec <= 0) delete deNode;
	else running = deNode;
	
	if (top->data->procList()->NodeCount() == 0
		&& (stackList.NodeCount() > 1 || ProcNode::Count() == 0))
	{
		if (P == top)
			P = (P->NextNode() ? P->NextNode() : stackList.GetBottom());
		auto del = top->data;
		top->parent->DeleteRequest(top);
		delete del;
	}

	if (ProcNode::Count() == 0)
		return;
	promote();
}

void promote()
{
	LinkedList<StackNode>::Node* pNext;
	LinkedList<StackNode>::Node* nextStack;

	if (!P->NextNode())
	{
		if (P->data->procList()->NodeCount() <= 1)
		{
			P = stackList.GetBottom();
			return;
		}
		stackList.Add(new StackNode());
		nextStack = stackList.GetTop();
		pNext = stackList.GetBottom();
	}
	else
	{
		nextStack = P->NextNode();
		pNext = P->NextNode();
	}

	ProcNode* promoteNode = P->data->procList()->Remove();

	nextStack->data->procList()->Add(promoteNode);
	promoteNode->isPromoted = true;

	if (P->data->procList()->NodeCount() == 0)
	{
		auto del = P->data;
		P->parent->DeleteRequest(P);
		delete del;
	}

	P = pNext;

	split_n_merge(P);
}

void split_n_merge(LinkedList<StackNode>::Node* stack)
{
	int threshold = ProcNode::Count() / StackNode::Count();
	int count = stack->data->procList()->NodeCount();
	if (count <= threshold)
		return;

	LinkedList<StackNode>::Node* nextStack;
	if (!stack->NextNode())
	{
		stackList.Add(new StackNode());
		nextStack = stackList.GetTop();
	}
	else
	{
		nextStack = stack->NextNode();
	}

	stack->data->procList()->Add(stack->data->procList()->Remove(count / 2));

	if (stack->data->procList()->NodeCount() == 0)
	{
		if (P == stack)
			P = (P->NextNode() ? P->NextNode() : stackList.GetBottom());
		auto del = stack->data;
		stack->parent->DeleteRequest(stack);
		delete del;
	}
	

	split_n_merge(nextStack);
}
#pragma endregion Dynamic_Queueing

#pragma region commandFunc
void shell(ProcNode::ProcInfo* proc)
{
	if (command.eof())
		return;
	string line;

	getline(command, line);

	printMtx.lock();
	cout << "prompt>" << line << endl;
	printMtx.unlock();

	exec(parse(line.c_str()));
}

void monitor(ProcNode::ProcInfo* proc)
{
	printMtx.lock();
	qeueuMtx.lock();
	cout << "Running: [";
	if (running != nullptr)
		cout << running->info.id << (running->info.type == processType::Foreground ? "F]" : "B]");
	else
		cout << "]";
	cout << endl << "---------------------------" << endl;

	cout << "DQ: ";

	for (LinkedList<StackNode>::Node* stack = stackList.GetBottom(); stack != nullptr; stack = stack->NextNode())
	{
		cout << (P == stack ? "P => [" : "     [");

		for (auto temp = stack->data->procList()->GetBottom(); temp != nullptr; temp = temp->NextNode())
		{
			cout << (temp->data->isPromoted ? "*" : "")
				<< temp->data->info.id
				<< (temp->data->info.type == processType::Foreground ? "F" : "B")
				<< (temp->NextNode() != nullptr ? " " : "");

			temp->data->isPromoted = false;
		}
		cout << "]";

		bool isBottom = false, isTop = false;
		if (stack == stackList.GetBottom())
			isBottom = true;
		if (stack == stackList.GetTop())
			isTop = true;

		cout << (isBottom || isTop ? "(" : "")
			<< (isBottom ? "bottom" : "")
			<< (isBottom && isTop ? "/" : "")
			<< (isTop ? "top" : "")
			<< (isBottom || isTop ? ")" : "");

		if (stack != stackList.GetTop())
			cout << endl << "    ";
	}

	cout << endl << "---------------------------" << endl;
	cout << "WQ: [";
	for (auto temp = WQ.GetBottom(); temp != nullptr; temp = temp->NextNode())
	{
		cout << temp->data->info.id
			<< (temp->data->info.type == processType::Foreground ? "F:" : "B:")
			<< temp->data->leftWait
			<< (temp->NextNode() != nullptr ? " " : "");
	}
	cout << "]" << endl;
	printMtx.unlock();
	qeueuMtx.unlock();
}

void echo(ProcNode::ProcInfo* proc)
{
	printMtx.lock();
	cout << proc->args[0] << endl;
	printMtx.unlock();
}

void gcd(ProcNode::ProcInfo* proc)
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

void prime(ProcNode::ProcInfo* proc)
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
		for (unsigned int j = 0; j < primes.size(); j++)
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

void sum(ProcNode::ProcInfo* proc)
{
	int _x = stoi(proc->args[0]), _m = stoi(proc->args[1]);

	vector<thread*> sumThread;
	int result = _x;

	for (int i = 0; i < _m; i++)
	{
		int end = (i + 1 == _m) ? _x : (_x / _m) * (i + 1);
		sumThread.push_back(new thread(sumTh, (_x / _m) * i, end, &result));
	}

	for (unsigned int i = 0; i < sumThread.size(); i++)
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
#pragma endregion commandFunc

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
			Make(command);
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
		Make(command);
	}

	free(*reader);

	free(args);
}

void Make(vector<string> args)
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

	for (unsigned int i = (type == processType::Foreground ? 1 : 2); i < args.size(); i++)
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
		ProcNode* newProc = new ProcNode(id++, type, d, p, sec);

		switch (proc)
		{
		case processList::echo:
			newProc->info.func = echo;
			break;
		case processList::dummy:
			newProc->info.func = nullptr;
			break;
		case processList::gcd:
			newProc->info.func = gcd;
			break;
		case processList::prime:
			newProc->info.func = prime;
			break;
		case processList::sum:
			newProc->info.func = sum;
			break;
		}

		newProc->info.args = _args;
		mtxEnqueue(newProc);
	}

}

void MakeTh(ProcNode* proc)
{
	if (proc->info.type == processType::Foreground)
	{
		fgMtx.lock();
		fg = new thread(ProcessRunner, proc->info, fg);
		fgMtx.unlock();
	}
	else
	{
		thread t(ProcessRunner, proc->info, nullptr);
		t.detach();
	}
}

void ProcessRunner(ProcNode::ProcInfo proc, thread* prevTh)
{
	if (prevTh != nullptr)
		prevTh->join();

	if (proc.func != nullptr)
		proc.func(&proc);

	if (prevTh != nullptr)
	{
		fgMtx.lock();
		if (fg == prevTh)
		{
			fg = nullptr;
		}
		delete prevTh;

		fgMtx.unlock();
	}
}