#include "TCResultFilter.hpp"

namespace smsc {
namespace test {
namespace util {

using namespace std;

TCResultStack::~TCResultStack()
{
	for (int i = 0; i < size(); i++)
	{
		const TCResult* res = (*this)[i];
		delete res;
	}
}

void TCResultStack::push_back(const TCResult* result)
{
	//игнорирую незаимплементированные test cases, которые возвращают NULL
	if (result)
	{
		vector<const TCResult*>::push_back(result);
	}
}

bool TCResultStack::operator== (const TCResultStack& stack) const
{
	if (size() < stack.size())
	{
		return false;
	}
	bool res = true;
	for (int i = 0; i < stack.size(); i++)
	{
		res &= *(*this)[i] == *stack[i];
	}
	return res;
}

bool TCResultStack::operator== (const TCResult& result) const
{
	return (size() == 1 && result == *(*this)[0]);
}

TCResultStackList::~TCResultStackList()
{
	for (int i = 0; i < size(); i++)
	{
		const TCResultStack* stack = (*this)[i];
		delete stack;
	}
}

ostream& operator<< (ostream& os, const TCResultStackList& stackList)
{
	for (int i = 0; i < stackList.size(); i++)
	{
		if (i > 0)
		{
			os << ",";
		}
		const TCResultStack& stack = *stackList[i];
		for (int j = 0; j < stack.size(); j++)
		{
			if (j > 0)
			{
				os << "->";
			}
			const TCResult& res = *stack[j];
			os << res;
		}
	}
	return os;
}

TCResultFilter::~TCResultFilter()
{
	for (TCMap::iterator it = resmap.begin(); it != resmap.end(); it++)
	{
		TCResultStackList* stackList = it->second;
		delete stackList;
	}
}

void TCResultFilter::addResult(const TCResult* result)
{
	//проверка
	if (!result)
	{
		return;
	}

	//зарегистрировать результат
	const char* tcId = result->getId();
	if (!resmap[tcId])
	{
		resmap[tcId] = new TCResultStackList();
	}

	//проверить является ли переданный результат test case отрицательным 
	if (result->value())
	{
		return;
	}

	//проверить нет ли уже идентичного test case
	TCResultStackList& stackList = *(resmap[tcId]);
	for (int i = 0; i < stackList.size(); i++)
	{
		const TCResultStack& stack = *(stackList[i]);
		if (stack == *result)
		{
			return;
		}
	}

	//добавить результат в список стеков
	TCResultStack* stack = new TCResultStack();
	stack->push_back(new TCResult(*result));
	stackList.push_back(stack);
}

void TCResultFilter::addResultStack(const TCResultStack& stack)
{
	//зарегистрировать результаты и проверить есть ли в переданном стеке
	//test case с отрицательным результатом
	int failedTC = -1;
	for (int i = 0; i < stack.size(); i++)
	{
		const TCResult* res = stack[i];
		const char* tcId = res->getId();
		if (!resmap[tcId])
		{
			resmap[tcId] = new TCResultStackList();
		}
		if (failedTC == -1 && !res->value())
		{
			failedTC = i;
		}
	}
	if (failedTC == -1)
	{
		return;
	}

	//проверить нет ли уже идентичного стека test case и
	//пометить, что результат по test case добавлялся
	const TCResult* failedRes = stack[failedTC];
	TCResultStackList& stackList = *(resmap[failedRes->getId()]);
	for (int i = 0; i < stackList.size(); i++)
	{
		const TCResultStack& _stack = *(stackList[i]);
		if (stack == _stack)
		{
			return;
		}
	}

	//добавить стек в список стеков
	TCResultStack* _stack = new TCResultStack();
	for (int i = 0; i <= failedTC; i++)
	{
		const TCResult* res = stack[i];
		_stack->push_back(new TCResult(*res));
	}
	stackList.push_back(_stack);
}

const TCResultStackList* TCResultFilter::getResults(const char* tcId)
{
	return resmap[tcId];
}

}
}
}

