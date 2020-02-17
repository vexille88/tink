// ~/cProject/algostuff.hpp

#ifndef ALGOSTUFF_HPP
#define ALGOSTUFF_HPP
// 常用标准库
#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <functional>
#include <numeric>
// 辅助库
#include <tuple>
#include <complex>
// 容器类
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>

//using namespace std;

template <class T>
inline void PRINT_ELEMENTS(const T& coll, const char* optcstr=" ")
{
	typename T::const_iterator pos;
	std::cout << optcstr;
	for(pos=coll.begin(); pos!=coll.end(); ++pos){
		std::cout << *pos << ' ';
	}
	std::cout << std::endl;
}

template <class T>
inline void INSERT_ELEMENTS(T& coll,int first,int last)
{
	for(int i=first; i<=last; ++i){
		coll.insert(coll.end(),i);
	}
}

#endif /*ALGOSTUFF_HPP*/
