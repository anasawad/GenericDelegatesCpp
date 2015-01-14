/************************************************************************/
/* Generic Delegates Implementation using variadic templates			*/
/************************************************************************/
#include <iostream>
#include <vector>
using namespace std;



template<typename TReturnType, typename... TParams>
class Delegate
{
	typedef TReturnType(*Callback)(void* pObj, TParams...);
public:
	Delegate(void* pObj, Callback pfnCallback)
		:m_pObj(pObj), m_pfnCallback(pfnCallback)
	{}

	TReturnType operator()(TParams ... paramList)const
	{
		return (*m_pfnCallback)(m_pObj, paramList ...);
	}

	bool operator==(const Delegate& pOther)const
	{
		return (this->m_pObj == pOther.m_pObj) && (this->m_pfnCallback == pOther.m_pfnCallback);
	}

	void Call(TParams... paramList)
	{
		(*m_pfnCallback)(m_pObj, paramList ...);
	}

private:

	Callback m_pfnCallback;
	void	*m_pObj;

};

/************************************************************************/
/* DelegateFactory Class: Factory class for Creating Delegate instance  */
/************************************************************************/
template<typename TReceiver, typename TReturnType, typename... TParams>
class DelegateFactory
{
public:
	template<TReturnType(TReceiver::*pfnCallback)(TParams...)>
	static TReturnType MemberFunctionCaller(void* pObj, TParams... paramList)
	{
		return (static_cast<TReceiver*>(pObj)->*pfnCallback)(paramList...);
	}

	template<TReturnType(*pfnCallback)(TParams...)>
	static TReturnType FreeFunctionCaller(void*, TParams... paramList)
	{
		return (pfnCallback)(paramList...);
	}

	template<TReturnType(TReceiver::*pfnCallback)(TParams...)>
	inline static Delegate<TReturnType, TParams...>MakeMemberFuncDelegate(TReceiver* pObj)
	{
		return Delegate<TReturnType, TParams...>(pObj, &DelegateFactory::MemberFunctionCaller<pfnCallback>);
	}

	template<TReturnType(*pfnCallback)(TParams...)>
	inline static Delegate<TReturnType, TParams...>MakeFreeFuncDelegate()
	{
		return Delegate<TReturnType, TParams...>(nullptr, &DelegateFactory::FreeFunctionCaller<pfnCallback>);
	}
};



template<typename TReceiver, typename TReturnType, typename... TParams>
DelegateFactory<TReceiver, TReturnType, TParams...>GetMemberFuncDelInstance(TReturnType(TReceiver::*)(TParams...))
{
	return DelegateFactory<TReceiver, TReturnType, TParams...>();
}

class no_type{};
template<typename TReturnType, typename... TParams>
DelegateFactory<no_type, TReturnType, TParams...>GetFreeFuncDelInstance(TReturnType(*pfnCallback)(TParams...))
{
	return DelegateFactory<no_type, TReturnType, TParams... >();
}

//Delegate instance for non-static member functions 
#define DELEGATE_FOR_MEMBERS(pfnCallback,pObj)(GetMemberFuncDelInstance(pfnCallback).MakeMemberFuncDelegate<pfnCallback>(pObj))

//Delegate instance for non-static free functions 
#define DELEGATE_FOR_FREE(pfnCallback)(GetFreeFuncDelInstance(pfnCallback).MakeFreeFuncDelegate<pfnCallback>())



template<typename TReturnType, typename... TParams>
class MulticastDelegate
{
public:
	MulticastDelegate(){}
	~MulticastDelegate()
	{
		m_observers.clear();
	}

	void operator()(TParams... paramList)
	{
		for (auto handler : m_observers)
		{
			handler.Call(paramList...);
		}
	}

	void operator+=(Delegate<TReturnType, TParams...>& del)
	{
		 Register(del);
	}
	void operator-=(Delegate<TReturnType, TParams...>& del)
	{
		 Unregister(del);
	}
	void Register(Delegate<TReturnType, TParams...>& del)
	{
		m_observers.push_back(del); 
	}
	void Unregister(Delegate<TReturnType, TParams...>& del)
	{
		m_observers.erase(del); 
	}
private:
	std::vector<Delegate<TReturnType, TParams...>>m_observers;
};

class ClassTest
{
public:
	int test_foo(int x)
	{
		printf("%d\n", x*x);
		return x*x;
	}
	int test_foo1(int x)
	{
		printf("%d\n", x + x);
		return x + x;
	}
	int bar(int x, int y, char a)
	{
		return a == 'a' ? x + y : x*y;
	}

};

int Free_Function(int x)
{
	return x;
}

void main(void)
{
	ClassTest temp;
	ClassTest temp1;
	

	auto d = DELEGATE_FOR_MEMBERS(&ClassTest::test_foo, &temp);
	
	auto d2 = DELEGATE_FOR_MEMBERS(&ClassTest::test_foo, &temp);

	auto d3 = MulticastDelegate<int, int>();
	d3 += DELEGATE_FOR_MEMBERS(&ClassTest::test_foo, &temp);
	if (d == d2)
	{
		printf("Yes");
	}
	
	d3(4);
}