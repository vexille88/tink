// ~/cProject/myasio.cpp
// g++ myasio.cpp -o myasio -lboost_system -lpthread -lboost_atomic
// gcc 6.3.0 boost 1.69.0
#define BOOST_ASIO_DISABLE_STD_CHRONO
#include <boost/chrono.hpp>
#include <boost/asio/steady_timer.hpp>
//#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/system/error_code.hpp>
#include <boost/lexical_cast.hpp>// 使用字符串转换功能
#include <iostream>
#include <exception>
#include <vector>

using namespace boost::asio;

class my_base_class_client
{
    typedef my_base_class_client this_type;// 简化类型定义
    typedef ip::address address_type;
    typedef ip::tcp::endpoint endpoint_type;
    typedef ip::tcp::socket socket_type;
    typedef ip::tcp::resolver resolver_type;
    typedef boost::shared_ptr<socket_type> sock_ptr;
    typedef std::vector<char> buffer_type;
private:
    io_service m_io;// io_service对象，不需要初始化
    buffer_type m_buff;// 接收缓冲区，在整个对象生命周期存在
    //endpoint_type m_ep;// TCP端点
    resolver_type m_resolver;// 网络解析器
    //sock_ptr m_sock;// 套接字智能指针
    resolver_type::iterator end;// end iterator
    steady_timer m_timer;
    streambuf m_request;
    streambuf m_response;
public:
    // 初始化IP地址和PORT端口
    my_base_class_client(const std::string& str_host, const std::string str_service, const int buff = 1024):m_buff(buff,0),m_resolver(m_io),m_timer(m_io)
    {
        //构造HTTP请求发送给服务器.  
        std::ostream requestPacket(&m_request);  
        requestPacket << "GET: " << "http://www.boost.org/" << " HTTP/1.0\r\n";  
        requestPacket << "Host: " << "www.boost.org" << "\r\n";  
        requestPacket << "Accept: */*\r\n";  
        requestPacket << "Connection: close\r\n\r\n"; 
        std::cout << m_request.size() << std::endl;
        streambuf::const_buffers_type cbuffers = m_request.data();
        std::string bufdata(buffers_begin(cbuffers),buffers_end(cbuffers));
        std::cout << bufdata << std::endl;
        start(str_host,str_service);// 启动异步连接
    }

    void run()
    {
        m_io.run();
    }

private:
    void start(const std::string& host, const std::string& port)
    {
        resolver_type::query query(host, port);
        //steady_timer m_timer(m_io);
        m_resolver.async_resolve(query,boost::bind(&this_type::resolve_handler,this,placeholders::error,placeholders::iterator));
        //m_timer.expires_from_now(boost::chrono::seconds(5));
        //m_timer.async_wait(boost::bind(&this_type::timer_handler,this,placeholders::error,m_sock));
    }

    void conn_handler(const boost::system::error_code& ec,sock_ptr sock,resolver_type::iterator iter)
    {
        // 不管网络连接是否成功，只要完成就删除延时定时器
        m_timer.cancel();
        /*
        if(ec)// 连接失败
        {
            std::cout << "Connect falure: " << ec.message() << std::endl;
        }

        else// 连接成功
        {
            std::cout << "connect success." << std::endl;
            std::cout << "receive from:" << sock->remote_endpoint().address() << std::endl;
            // 读取网络
            //sock->async_read_some(buffer(m_buff),boost::bind(&this_type::read_handler,this,placeholders::error,sock));// TCP
        }
        
        if(iter != end)
        {   // 递归调用，继续连接下一个服务器的端点
            sock->close();
            // 建立一个新的连接对象
            sock_ptr m_sock(new socket_type(m_io));
            //steady_timer m_timer(m_io);
            m_sock->async_connect(*iter,boost::bind(&this_type::conn_handler,this,placeholders::error,m_sock,++iter));// 异步连接   
            m_timer.expires_from_now(boost::chrono::seconds(5));
            m_timer.async_wait(boost::bind(&this_type::timer_handler,this,placeholders::error,m_sock));
        }
        else
        {
            sock->close();
            //std::cout << "endpoint: " << iter->endpoint() << std::endl; // 抛出异常
            std::cout << "... iterator end." << std::endl;
        }
        */
        
        if(!ec)// 连接服务器成功，接着从服务器接收数据
        {
            std::cout << "connect success." << std::endl;
            std::cout << "receive from " << sock->remote_endpoint().address() << std::endl;
            //m_sock->async_read_some(buffer(m_buff),boost::bind(&this_type::read_handler,this,placeholders::error,m_sock));// TCP
            //sock->async_receive_from(buffer(m_buff),boost::bind(&this_type::read_handler,this,placeholders::error,sock));// UDP 
            async_write(*sock,m_request,boost::bind(&this_type::write_handler,this,placeholders::error,placeholders::bytes_transferred,sock));
        }
        else if(iter != end)
        {   // 递归调用，继续连接下一个服务器的端点
            sock->close();
            // 建立一个新的连接对象
            sock_ptr m_sock(new socket_type(m_io));
            //steady_timer m_timer(m_io);
            m_sock->async_connect(*iter,boost::bind(&this_type::conn_handler,this,placeholders::error,m_sock,++iter));// 异步连接   
            m_timer.expires_from_now(boost::chrono::seconds(5));
            m_timer.async_wait(boost::bind(&this_type::timer_handler,this,placeholders::error,m_sock));
        }
        else
        {
            sock->close();
            //std::cout << "endpoint: " << iter->endpoint() << std::endl; // 抛出异常
            std::cout << "... iterator end." << std::endl;
        }
        
    }

    void write_handler(const boost::system::error_code& ec, std::size_t bytes, sock_ptr sock)
    {
        if(ec)
        {
            std::cout << std::endl;
            std::cout << "Write error: " << ec.message() << std::endl;
            return;
        }

        async_read_until(*sock,m_response,"\r\n\r\n",boost::bind(&this_type::read_handler,this,placeholders::error,placeholders::bytes_transferred));

    }
    void read_handler(const boost::system::error_code& ec, size_t bytes)
    {
        if(ec)// 接收完所有数据时
        {
            std::cout << std::endl;
            std::cout << "Read error: " << ec.message() << std::endl;
            return;
        }
        std::ostringstream str;
        str << &m_response;
        std::cout << str.str() << std::endl;
        //std::istream responsePacket(&m_response);
        //std::string http_version;
        //responsePacket >> http_version;
        //unsigned int status_code;
        //responsePacket >> status_code;
        //std::cout << "response: " << http_version << " : " << status_code << std::endl;
        // 输出读取结果
        //std::cout << &m_buff[0];
        // 缓冲区容量不足时，采用递归调用，实现循环异步读取数据
        //sock->async_read_some(buffer(m_buff),boost::bind(&this_type::read_handler,this,placeholders::error,sock));// TCP
        //sock->async_receive_from(buffer(m_buff),boost::bind(&this_type::read_handler,this,placeholders::error,sock));// UDP
    }

    // 异步解析网络后，得到endpoint(resolver::iterator)进一步处理后续网络连接
    void resolve_handler(const boost::system::error_code& ec,resolver_type::iterator iter)
    {
        
        if(ec)// 网络解析失败
        {
            std::cout << "can't resolver." << std::endl;
            throw boost::system::system_error(ec);
        }
        
        // 检查网络解析的结果，有几个IP地址
        std::cout << "resolver success." << std::endl;
        auto iter1 = iter;
        int i = 1;
        decltype(iter) end1;
        while(iter1 != end1)
        {
            std::cout << "endpoint: " << iter1->endpoint() << std::endl;
            std::cout << "iterator number: " << i << " -----------------" << std::endl;
            ++i;
            ++iter1;
        }
        // 结束，获得IP地址后开始连接网络
        
        //std::cout << "endpoint: " << iter->endpoint() << std::endl;
        // 异步网络连接
        sock_ptr m_sock(new socket_type(m_io));
        //steady_timer m_timer(m_io);
        m_sock->async_connect(*iter,boost::bind(&this_type::conn_handler,this,placeholders::error,m_sock,++iter));
        m_timer.expires_from_now(boost::chrono::seconds(5));
        m_timer.async_wait(boost::bind(&this_type::timer_handler,this,placeholders::error,m_sock));
    }

    void timer_handler(const boost::system::error_code& ec,sock_ptr sock)
    {
        if(!ec)
        {
            std::cout << "Connect timeout: " << ec.message() << std::endl;
            //std::cout << "not connected" << std::endl;
            sock->close();
        }
        
    }
};

// 单元测试（test）
int main(int argc, char* argv[])
try
{
    std::cout << "client start..." << std::endl;
    //my_base_class_client cl("127.0.0.1","6688");
    my_base_class_client cl("edu.ntp.org.cn","ntp");
    cl.run();
}
catch(std::exception e)
{
    std::cout << e.what() << std::endl;
}