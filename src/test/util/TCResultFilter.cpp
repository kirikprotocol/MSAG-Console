#include "TCResultFilter.hpp"

namespace smsc {
namespace test {
namespace util {

using namespace std;

TCResultStack::~TCResultStack()
{
	for (int i = 0; i < size(); i++)
	{
		TCResult* res = (*this)[i];
		delete res;
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

TCResultFilter::~TCResultFilter()
{
	for (TCMap::iterator it = resmap.begin(); it != resmap.end(); it++)
	{
		TCValue* tcVal = it->second;
		for (int i = 0; i < tcVal->tcStacks.size(); i++)
		{
			TCResultStack* stack = tcVal->tcStacks[i];
			delete stack;
		}
		delete tcVal;
	}
}

void TCResultFilter::registerTC(const string& tcId,
	const string& tcDescription)
{
	if (resmap[tcId] == NULL)
	{
		resmap[tcId] = new TCValue(tcDescription);
	}
}

inline TCResultFilter::TCValue& TCResultFilter::getTCValue(const string& tcId)
{
	TCValue* val = resmap[tcId];
	if (val == NULL)
	{
		val = new TCValue("-");
		resmap[tcId] = val;
	}
	return *val;
}

void TCResultFilter::addResult(TCResult& result)
{
	//пометить test case как использованный
	TCValue& tcVal = getTCValue(result.getId());
	tcVal.used = true;

	//проверить является ли переданный результат test case отрицательным 
	if (result.value())
	{
		return;
	}

	//проверить нет ли уже идентичного test case
	for (int i = 0; i < tcVal.tcStacks.size(); i++)
	{
		TCResultStack& stack = *(tcVal.tcStacks[i]);
		if (stack == result)
		{
			return;
		}
	}

	//добавить результат в список стеков
	TCResultStack* stack = new TCResultStack();
	stack->push_back(new TCResult(result));
	tcVal.tcStacks.push_back(stack);
}

void TCResultFilter::addResultStack(TCResultStack& stack)
{
	//пометить все test cases из стека как использованные
	for (int i = 0; i < stack.size(); i++)
	{
		TCValue& tcVal = getTCValue(stack[i]->getId());
		tcVal.used = true;
	}

	//проверить есть ли в переданном стеке test case с отрицательным 
	//результатом
	TCResult* failedTCResult = NULL;
	for (int i = 0; i < stack.size(); i++)
	{
		TCResult& res = *stack[i];
		if (!res.value())
		{
			failedTCResult = stack[i];
			break;
		}
	}
	if (failedTCResult == NULL)
	{
		return;
	}

	//проверить нет ли уже идентичного стека test case и
	//пометить, что результат по test case добавлялся
	TCValue& tcVal = getTCValue(failedTCResult->getId());
	for (int i = 0; i < tcVal.tcStacks.size(); i++)
	{
		TCResultStack& _stack = *(tcVal.tcStacks[i]);
		if (stack == _stack)
		{
			return;
		}
	}

	//добавить стек в список стеков
	TCResultStack* _stack = new TCResultStack();
	for (int i = 0; i < stack.size(); i++)
	{
		TCResult& res = *stack[i];
		_stack->push_back(new TCResult(res));
		if (!res.value())
		{
			break;
		}
	}
	tcVal.tcStacks.push_back(_stack);
}

}
}
}

