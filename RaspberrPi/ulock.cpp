// ~/cProject/ulock.cpp
// g++ ulock.cpp -o ulock -lboost_system -lboost_thread -lpthread
// ./ulock
#include <iostream> 
//#include <pthread.h>
#include <boost/asio.hpp> 
#include <boost/thread.hpp> 
#include <boost/chrono.hpp>
#include <boost/thread/thread_guard.hpp>

//#include <boost/atomic.hpp>
//#include <boost/intrusive_ptr.hpp>

void to_interrupt(int m)
try
{
	for (int i =0; i < m; ++i)
	{
		boost::chrono::milliseconds ms(100);
		boost::this_thread::sleep_for(ms);
		std::cout << i << std::endl;
		//boost::this_thread::interruption_point();
	}
}
catch(const boost::thread_interrupted& )
{
	std::cout << "thread_interrupted" << std::endl;
}

int main()
{
	boost::thread t(to_interrupt,10);
	//boost::thread t2(dummy,50000);
	boost::chrono::milliseconds s(700);
	boost::this_thread::sleep_for(s);

	t.interrupt();
	assert(t.interruption_requested());

	t.join();

	return 0;
}