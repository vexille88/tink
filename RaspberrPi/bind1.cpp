// ~/cProject/bind1.cpp
// g++ bind1.cpp -o bind1

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <iostream>
#include <vector>
#include <algorithm>

struct point
{
	int x, y;
	point(int a = 0,int b = 0):x(a),y(b) {}
	void print()
	{
		std::cout << "(" << x << "," << y <<")" <<std::endl;
	}
};

class demo_class
{
private:
	typedef boost::function<void(int)> func_t;// function 类型定义
	func_t func;// 声明function对象
	int n;
public:
	demo_class(int i):n(i) {}
	template<typename CallBack>
	void accept(CallBack f)
	{	func = f;	}//回调函数赋值给function对象
	void run()
	{	func(n);	}//运行回调函数
};

void call_back_func(int i)
{
	std::cout << "call_back_func:" << i * 2 << std::endl;
}

int main()
{
	std::vector<point> v(10);
	std::vector<int> v2(10);
	/*for_each(v.begin(), v.end(), boost::bind(&point::print, _1));
	transform(v.begin(),v.end(),v2.begin(),boost::bind(&point::x, _1));
	for (auto x : v2)
		std::cout << x << ",";
	std::cout << std::endl;*/
	demo_class dc(10);
	dc.accept(call_back_func);
	dc.run();

	return 0;

}