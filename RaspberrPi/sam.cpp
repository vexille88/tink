// ~/cProject/sam.cpp
// 常用标准库
#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
// 辅助库
#include <tuple>
#include <complex>
// 容器类
#include <vector>
#include <set>
#include <deque>

using namespace std;


class PrintInt {
public:
	void operator () (int elem) const {
		cout << elem << ' ';
	}
};

void square1(int& elem)
{
	elem = elem * elem;
}
int square2(int elem)
{
	return elem * elem;
}

int main()
{
	/*tuple<string,int,int,complex<double>> t;
	tuple<int,float,string> t1{41,6.3,"nico"};
	cout << get<0>(t1) << " ";
	cout << get<1>(t1) << " ";
	cout << get<2>(t1) << " ";
	cout << endl;
	vector<int> coll;
	for (int i=1; i<9; ++i) {
		coll.push_back(i);
	}
	for_each (coll.begin(), coll.end(), PrintInt());
	cout << endl;*/
	std::vector<int> v(100);
	//v.reserve(100);
	cout << v.empty() << endl;
	cout << v.size() << endl;
	cout << v.max_size() << endl;
	cout << v.capacity() << endl;

	set<int,greater<int>> coll1;
	deque<int> coll2;
	vector<int> coll3;

	for (int i=1; i<=9; ++i) {
		coll1.insert(i);
		coll3.push_back(i);
	}
	// 利用仿函数实现迭代输出
	for_each(coll3.begin(),coll3.end(),PrintInt());
	cout << endl;
	for_each(coll3.begin(),coll3.end(),square1);
	//transform(coll3.begin(),coll3.end(),coll3.begin(),square2);
	// 利用流迭代器实现输出
	copy(coll3.begin(),coll3.end(),ostream_iterator<int>(cout," "));
	cout << endl;

	int num = coll1.erase(3);
	cout << num << endl;
	copy(coll1.begin(),coll1.end(),ostream_iterator<int>(cout," "));
	cout << endl;

	transform(coll1.begin(),coll1.end(),back_inserter(coll2),bind2nd(multiplies<int>(),10));
	for_each(coll2.begin(),coll2.end(),PrintInt());
	cout << endl;


	return 0;
}

