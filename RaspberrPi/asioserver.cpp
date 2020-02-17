// ~/cProject/asioserver.cpp
// gcc 6.3.0 boost 1.69.0
// g++ asioserver.cpp -o asioserver -lboost_system -lpthread
#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <iostream>
#include <exception>

using namespace boost::asio;

class my_basic_class_server // 把异步通信封装成基类，能实现侦听和连接
{
    typedef my_basic_class_server this_type; // 简化类型定义
    typedef ip::tcp::acceptor acceptor_type;
    typedef ip::tcp::endpoint endpoint_type;
    typedef ip::tcp::socket socket_type;
    typedef boost::shared_ptr<socket_type> sock_ptr;
private: // 必须的两个对象
    io_service m_io;
    acceptor_type m_acceptor;
    streambuf m_buf;
public:
    my_basic_class_server(const int m_port):m_acceptor(m_io,endpoint_type(ip::tcp::v4(),m_port))
    {
        accept();  // 立即启动异步服务
    }

    void run()
    {
        m_io.run();
    }

    void accept() // void start_accept()
    {
        sock_ptr sock(new socket_type(m_io));// 智能指针shared_ptr，为了传递给handler，其生命周期是整个类对象的存在期
        // 侦听连接请求，并建立连接。*sock解指针，这里传递一个引用
        m_acceptor.async_accept(*sock,boost::bind(&this_type::accept_handler,this,placeholders::error,sock));
    }

    void accept_handler(const boost::system::error_code& ec, sock_ptr sock)
    {
        if(ec) // 检测错误码！sock是智能指针，在其生命周期结束会自动销毁
        {
            //delete sock;
            return;
        }
        // 自定义部分----------------------------
        std::cout << "client: " << sock->remote_endpoint().address() << std::endl;
        async_read_until(*sock,m_buf,"\r\n\r\n",boost::bind(&this_type::read_handler,this,placeholders::error,placeholders::bytes_transferred,sock));
        //sock->async_write_some(buffer("hello asio! who are you?"),bind(&this_type::write_handler,this,placeholders::error));
        //sock->async_send_to(buffer("hello asio! who ar you?"),bind(&this_type::write_handler,this,placeholders::error));
        // ---------------------------------------
        accept(); // 再次启动异步服务
    }
    // 自定义功能实现---------------------------
    void read_handler(const boost::system::error_code& ec,std::size_t bytes,sock_ptr sock)
    {
        if(ec)
        {
            std::cout << std::endl;
            std::cout << "Read error: " << ec.message() << std::endl;
            return;
        }
        async_write(*sock,m_buf,boost::bind(&this_type::write_handler,this,placeholders::error,placeholders::bytes_transferred));
    }
    void write_handler(const boost::system::error_code& ec,std::size_t bytes)
    {
        if(ec)
        {
            std::cout << std::endl;
            std::cout << "Write error: " << ec.message() << std::endl;
            return;
        }
        std::cout << "send msg complete!" << std::endl;
    }
};


// 单元测试（test）
int main()
try
{
    std::cout << "server start..." << std::endl;
    my_basic_class_server svr(6688);
    svr.run();
}
catch(std::exception& e)
{
    std::cout << e.what() << std::endl;
}

/* // g++ asioserver.cpp -o asio -lboost_system -lpthread -lboost_chrono
#define BOOST_ASIO_DISABLE_STD_CHRONO
#include <iostream>
//#include <chrono>
//#include <boost/thread.hpp>
//#include <boost/system/error_code.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/chrono.hpp>
#include <boost/asio/steady_timer.hpp>

using namespace boost::asio;
//using namespace std;

class timer_with_func
{
    typedef timer_with_func this_type;
private:
    int m_count = 0;
    int m_count_max = 0;
    boost::function<void()> m_f;
    steady_timer m_t;

public:
    template<typename F>
    timer_with_func(io_service& io, int x, F func):m_count_max(x),m_f(func),m_t(io,boost::chrono::milliseconds(200))
    {init();}
private:
    void init()
    {
        m_t.async_wait(boost::bind(&this_type::handler,this,placeholders::error));
    }
    void handler(const boost::system::error_code&)
    {
        if(m_count++ >= m_count_max)
            {return;}
        m_f();
        m_t.expires_from_now(boost::chrono::seconds(1));
        m_t.async_wait(boost::bind(&this_type::handler,this,placeholders::error));
    }
};

int main()
{
    io_service io;

    timer_with_func t1(io, 5, []{std::cout << "hello timer1!" << std::endl;});
    timer_with_func t2(io, 5, []{std::cout << "hello timer2!" << std::endl;});

    io.run();
    return 0;
}
*/