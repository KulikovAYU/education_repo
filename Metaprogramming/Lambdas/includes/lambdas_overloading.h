
#include <iostream>

#if ((defined(_MSVC_LANG) && _MSVC_LANG <= 201402L)) ///std:c++14

template<typename Func, typename ...Funcs>
struct overload : Func, overload<Funcs...>
{
	using Func::operator ();
	using overload<Funcs...>::operator ();
	overload(Func&& f, Funcs&& ... funcs) : Func(std::move(f)), overload<Funcs...>(std::move(funcs)...){}
};

template<typename Func>
struct overload <Func> : Func
{
	using Func::operator ();
	overload(Func&& f) : Func(std::move(f)){}
};

template<typename ...Funcs>
auto makeOverload(Funcs&&... funcs)
{
	return overload<Funcs...>(std::forward<Funcs>(funcs) ...);
}

#else if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)) \\C++17
#define CPP_17_OR_GREATER_IMPL

template<typename ...Funcs>
struct overload :public Funcs...
{
	using Funcs::operator ()...;
};

template<typename ...Funcs>
overload(Funcs...)->overload<Funcs...>;

#endif

int main()
{
	
	
	#ifdef CPP_17_OR_GREATER_IMPL
	auto funcs_ = overload([](int) {std::cout << "1" << std::endl; },
						   [](double) {std::cout << "2" << std::endl; }
	);
	#else
	auto funcs_ = makeOverload([](int) {std::cout << "1" << std::endl; },
							   [](double) {std::cout << "2" << std::endl; }
	);
	#endif // CPP_17_OR_GREATER_IMPL
	
	
	funcs_(1);
	funcs_(1.0);

}