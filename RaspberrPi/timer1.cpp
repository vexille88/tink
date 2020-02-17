// 标准库
#include <iostream> 
#include <ctime>

// Boost库，编译时需要链接的库：
// g++ timer1.cpp -o timer1 -lboost_system -lboost_chrono
//#include <boost/timer.hpp>
//#include <boost/progress.hpp>
#include <boost/chrono.hpp>
#include <boost/timer/timer.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>



int main()
{
	auto tp = boost::chrono::system_clock::now();
	auto t = boost::chrono::system_clock::to_time_t(tp);
	std::cout << std::ctime(&t) << std::endl;
	boost::gregorian::date d1(2019,4,12);
	boost::gregorian::date d2(boost::gregorian::not_a_date_time);
	boost::gregorian::date d3(boost::gregorian::max_date_time);
	boost::gregorian::date d4(boost::gregorian::min_date_time);
	std::tm t1 = to_tm(d1);
	std::cout << t1.tm_year << "/" << t1.tm_mon << "/" <<t1.tm_mday << std::endl;
	std::cout << d2 << std::endl;
	std::cout << d3 << std::endl;
	std::cout << d4 << std::endl;
	return 0;
}