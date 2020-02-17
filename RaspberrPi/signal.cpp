// ~/cProject/signal.cpp
// g++ signal.cpp -o signal

#include <boost/signals2.hpp> 
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/lock_factories.hpp>
#include <iostream> 
#include <utility>
#include <vector>
#include <numeric>
#include <stack>

class buffer
{
private:
  boost::mutex mu;

  boost::condition_variable_any cond_put;
  boost::condition_variable_any cond_get;

  std::stack<int> stk;
  int un_read, capacity;

  bool is_full()
  { return un_read == capacity;  }
  bool is_empty()
  { return un_read == 0;  }
public:
  buffer(size_t n):un_read(0),capacity(n) {}
  void put(int x)
  {
    {
      auto lock = boost::make_unique_lock(mu);
      for(;is_full();)
      {
        std::cout << "full waiting... " << std::endl;
        cond_put.wait(lock);
      }
      stk.push(x);
      ++un_read;
    }
    cond_get.notify_one();
  }

  void get(int *x)
  {
    {
      auto lock = boost::make_unique_lock(mu);
      for(;is_empty();)
      {
        std::cout << "empty waiting... " << std::endl;
        cond_get.wait(lock);
      }
      --un_read;
      *x = stk.top();
      stk.pop();
    }
    cond_put.notify_one();
  }
};

buffer buf(5);

void producer(int n)
{
  for(int i = 0;i < n; ++i)
  {
    std::cout << "put " << i << std::endl;
    buf.put(i);
  }
}

void consumer(int n)
{
  int x;
  for(int i = 0;i < n; ++i)
  {
    buf.get(&x);
    std::cout << "get " << x << std::endl;
  }
}

int main()
{
  boost::thread_group tg;
  tg.create_thread(boost::bind(producer,20));

  tg.create_thread(boost::bind(consumer,10));
  tg.create_thread(boost::bind(consumer,10));

  tg.join_all();

  return 0;
}