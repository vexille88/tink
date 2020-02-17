// from ripple project
//------------------------------------------------------------------------------
/*
    This file is part of rippled: https://github.com/ripple/rippled
    Copyright (c) 2012, 2013 Ripple Labs Inc.

    Permission to use, copy, modify, and/or distribute this software for any
    purpose  with  or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

#include "SNTPClock.h"
//#include <ripple/basics/Log.h>
//#include <ripple/basics/random.h>
//#include <ripple/beast/core/CurrentThreadName.h>
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <cmath>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <random>
#include <iostream>
#include <functional>

namespace ripple {

static uint8_t SNTPQueryData[48] =
{ 0x1B, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

using namespace std::chrono_literals;
// NTP query frequency - 4 minutes
auto constexpr NTP_QUERY_FREQUENCY = 4min;

// NTP minimum interval to query same servers - 3 minutes
auto constexpr  NTP_MIN_QUERY = 3min;

// NTP sample window (should be odd)
#define NTP_SAMPLE_WINDOW       9

// NTP timestamp constant
auto constexpr NTP_UNIX_OFFSET = 0x83AA7E80s;

// NTP timestamp validity (14min)
auto constexpr NTP_TIMESTAMP_VALID = (NTP_QUERY_FREQUENCY + NTP_MIN_QUERY) * 2;

// SNTP packet offsets
#define NTP_OFF_INFO            0
#define NTP_OFF_ROOTDELAY       1
#define NTP_OFF_ROOTDISP        2
#define NTP_OFF_REFERENCEID     3
#define NTP_OFF_REFTS_INT       4
#define NTP_OFF_REFTS_FRAC      5
#define NTP_OFF_ORGTS_INT       6
#define NTP_OFF_ORGTS_FRAC      7
#define NTP_OFF_RECVTS_INT      8
#define NTP_OFF_RECVTS_FRAC     9
#define NTP_OFF_XMITTS_INT      10
#define NTP_OFF_XMITTS_FRAC     11

class SNTPClientImp
    : public SNTPClock
{
private:
    template <class Duration>
        using sys_time = std::chrono::time_point<clock_type, Duration>;

    using sys_seconds = sys_time<std::chrono::seconds>;

    struct Query
    {
        bool replied;
        sys_seconds sent;
        std::uint32_t nonce;

        explicit Query (sys_seconds j = sys_seconds::max())
            : replied (false)
            , sent (j)
        {
        }
    };

    //beast::Journal j_;
    std::mt19937 rng;// 随机生成器
    std::mutex mutable mutex_;
    std::thread thread_;
    boost::asio::io_service io_service_;
    boost::optional<
        boost::asio::io_service::work> work_;

    std::map <boost::asio::ip::udp::endpoint, Query> queries_;// endpoint是指针类型
    boost::asio::ip::udp::socket socket_;
    boost::asio::basic_waitable_timer<std::chrono::system_clock> timer_;
    boost::asio::ip::udp::resolver resolver_;
    std::vector<std::pair<std::string, sys_seconds>> servers_;
    std::chrono::seconds offset_;
    sys_seconds lastUpdate_;
    std::deque<std::chrono::seconds> offsets_;
    std::vector<uint8_t> buf_;// 接收到的数据
    boost::asio::ip::udp::endpoint ep_; // remote endpoint

public:
    using error_code = boost::system::error_code;

    explicit
    SNTPClientImp ()
        : work_(io_service_)
        , socket_ (io_service_,boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(),0))
        , timer_ (io_service_)
        , resolver_ (io_service_)
        , offset_ (0)
        , lastUpdate_ (sys_seconds::max())
        , buf_ (256)
    {
        rng.seed(std::random_device()());
    }

    ~SNTPClientImp () override
    {   /*
        if (thread_.joinable())
        {
            error_code ec;
            timer_.cancel(ec);
            socket_.cancel(ec);
            work_ = boost::none;
            //thread_.join();
        }
        */
    }

    //--------------------------------------------------------------------------

    void
    doRun (const std::vector<std::string>& servers) override
    {
        std::vector<std::string>::const_iterator it = servers.begin ();

        if (it == servers.end ())
        {
            std::cout <<
                "SNTP: no server specified";
            return;
        }

        {
            std::lock_guard<std::mutex> lock (mutex_);
            for (auto const& item : servers)
            {
                // 打印SNTP服务器域名
                //std::cout << item << std::endl;
                servers_.emplace_back(
                    item, sys_seconds::max());
            }
            // 打印SNTP服务器域名，及时间最大度量
            //for (auto const& item : servers_)
            //	std::cout << item.first <<" : " << item.second.time_since_epoch().count() << std::endl;
        }

        using namespace boost::asio;
        queryAll();
        
        //socket_.open (ip::udp::v4 ());
        /*
        socket_.async_receive_from (buffer (buf_, 256), //256
            ep_, std::bind(
                &SNTPClientImp::onRead, this,
                    std::placeholders::_1,
                        std::placeholders::_2));
        //timer_.expires_from_now(NTP_QUERY_FREQUENCY);
        //timer_.async_wait(std::bind(
        //    &SNTPClientImp::onTimer, this,
        //        std::placeholders::_1));

        */
        std::cout << "SNTP: complete doRun()" << std::endl;
        // VFALCO Is it correct to launch the thread
        //        here after queuing I/O?
        //
        //thread_ = std::thread(&SNTPClientImp::doRun, this);
        //doRun();
    }

    time_point
    now() const override
    {
        std::lock_guard<std::mutex> lock (mutex_);
        using namespace std::chrono;
        auto const when = time_point_cast<seconds>(clock_type::now());
        if ((lastUpdate_ == sys_seconds::max()) ||
                ((lastUpdate_ + NTP_TIMESTAMP_VALID) <
                  time_point_cast<seconds>(clock_type::now())))
            return when;
        return when + offset_;
    }

    duration
    offset() const override
    {
        std::lock_guard<std::mutex> lock (mutex_);
        return offset_;
    }

     void run()
    {
        //beast::setCurrentThreadName("rippled: SNTPClock");
        std::cout << "io_service.run() start >>> " << std::endl;
        io_service_.run();
    }


    //--------------------------------------------------------------------------
private:
   
    void
    onTimer (error_code const& ec)
    {
        using namespace boost::asio;
        if (ec == error::operation_aborted)
            return;
        if (ec)
        {
            std::cout <<
                "SNTPClock::onTimer: " << ec.message();
            return;
        }

        doQuery ();
        //queryAll();
        timer_.expires_from_now(NTP_QUERY_FREQUENCY);
        timer_.async_wait(std::bind(
            &SNTPClientImp::onTimer, this,
                std::placeholders::_1));
    }

    void
    onRead (error_code const& ec, std::size_t bytes_xferd)
    {
        using namespace boost::asio;
        using namespace std::chrono;
        std::cout << "received data!" << " ec = " << ec << std::endl;
        if (ec == error::operation_aborted)
            return;

        // VFALCO Should we return on any error?
        /*
        if (ec)
            return;
        */
        
        std::cout << "SNTP: Packet from " << ep_ << std::endl;
        /*
        if (!ec)
        {
            std::cout <<
                "SNTP: Packet from " << ep_;
            std::lock_guard<std::mutex> lock (mutex_);
            auto const query = queries_.find (ep_);
            if (query == queries_.end ())
            {
                std::cout <<
                    "SNTP: Reply from " << ep_ << " found without matching query";
            }
            else if (query->second.replied)
            {
                std::cout <<
                    "SNTP: Duplicate response from " << ep_;
            }
            else
            {
                query->second.replied = true;

                if (time_point_cast<seconds>(clock_type::now()) > (query->second.sent + 1s))
                {
                    std::cout <<
                        "SNTP: Late response from " << ep_;
                }
                else if (bytes_xferd < 48)
                {
                    std::cout <<
                        "SNTP: Short reply from " << ep_ <<
                            " (" << bytes_xferd << ") " << buf_.size ();
                }
                else if (reinterpret_cast<std::uint32_t*>(
                        &buf_[0])[NTP_OFF_ORGTS_FRAC] !=
                            query->second.nonce)
                {
                    std::cout <<
                        "SNTP: Reply from " << ep_ << "had wrong nonce";
                }
                else
                {
                    processReply ();
                }
            }
        }
		*/
		if(!ec && bytes_xferd > 0)
		{
			for(auto iter : buf_)
        	   std::cout << iter;
        	std::cout << std::endl;
        	std::cout << "buf_.size() = " << bytes_xferd << std::endl;
        	//return;
            //doQuery ();
            //timer_.expires_from_now(std::chrono::seconds(10));
            //timer_.wait();
		}
        /*
		else
		{
			doQuery();	
		}
        */
		
        
    }

    //--------------------------------------------------------------------------

    void addServer (std::string const& server)
    {
        std::lock_guard<std::mutex> lock (mutex_);
        servers_.push_back (std::make_pair (server, sys_seconds::max()));
    }

    void queryAll ()
    {
        while (doQuery ())
        {
            std::cout << "finished doQuery()" << std::endl;
        }
    }

    bool doQuery ()
    {
        std::lock_guard<std::mutex> lock (mutex_);
        auto best = servers_.end ();

        for (auto iter = servers_.begin (), end = best;
                iter != end; ++iter)
            // 针对多个域名，首次依次从尾至首选用，然后从第二轮开始根据时间“最早（小）”选用
            if ((best == end) || (iter->second == sys_seconds::max()) ||
                                 (iter->second < best->second))
            {
            	best = iter;
            	std::cout << best->first <<" : " << best->second.time_since_epoch().count() << std::endl;
            }
                

        if (best == servers_.end ())
        {
            std::cout <<
                "SNTP: No server to query" << std::endl;
            return false;
        }

        using namespace std::chrono;
        auto now = time_point_cast<seconds>(clock_type::now());
        /*
        if ((best->second != sys_seconds::max()) && ((best->second + NTP_MIN_QUERY) >= now))
        {
            std::cout <<
                "SNTP: All servers recently queried" << std::endl;
            return false;
        }
        */
        best->second = now;

        boost::asio::ip::udp::resolver::query query(
            boost::asio::ip::udp::v4 (), best->first, "ntp");
        resolver_.async_resolve (query, std::bind (
            &SNTPClientImp::resolveComplete, this,
                std::placeholders::_1,
                    std::placeholders::_2));
        std::cout <<
            "SNTPClock: Resolve pending for " << best->first << std::endl;
        return true;
    }

    void resolveComplete (error_code const& ec,
        boost::asio::ip::udp::resolver::iterator it)
    {
        using namespace boost::asio;
        if (ec == error::operation_aborted)
            return;
        if (ec)
        {
            std::cout <<
                "SNTPClock::resolveComplete: " << ec.message();
            return;
        }
        // 解析成功，至少有一个IP地址
        assert (it != ip::udp::resolver::iterator());

        auto sel = it;
        int i = 1;
        // 随机选择一个IP地址
        while (it != ip::udp::resolver::iterator())
        {
            std::cout << it->endpoint() << "|| i = " << i << std::endl;
            // 设置离散均匀分布产生随机整数
            std::uniform_int_distribution<std::mt19937::result_type> dist(0,i++);
            std::cout << "random: " << dist(rng) << std::endl;
            // 选择抽中为0的那个IP地址
            if (dist(rng) == 0)
                sel = it;
            ++it;
        }
        // 迭代器sel是一个指针，指向endpoit对象
        if (sel != ip::udp::resolver::iterator ())
        {
            std::lock_guard<std::mutex> lock (mutex_);
            Query& query = queries_[*sel];// 插入一个key为*sel的元素，并返回其值的引用；ntp服务器访问记录
            using namespace std::chrono;
            auto now = time_point_cast<seconds>(clock_type::now());

            if ((query.sent == now) || ((query.sent + 1s) == now))
            {
                // This can happen if the same IP address is reached through multiple names
                std::cout <<
                    "SNTP: Redundant query suppressed";
                return;
            }

            query.replied = false;
            query.sent = now;
            std::uniform_int_distribution<std::uint32_t> dist;// 产生随机数
            query.nonce = dist(rng);
            std::cout << query.nonce << std::endl;
            // The following line of code will overflow at 2036-02-07 06:28:16 UTC
            //   due to the 32 bit cast.
            reinterpret_cast<std::uint32_t*> (SNTPQueryData)[NTP_OFF_XMITTS_INT] =
                static_cast<std::uint32_t>((time_point_cast<seconds>(clock_type::now()) +
                                            NTP_UNIX_OFFSET).time_since_epoch().count());
            reinterpret_cast<std::uint32_t*> (SNTPQueryData)[NTP_OFF_XMITTS_FRAC] = query.nonce;

            //socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
            //socket_.bind(*sel);
            socket_.async_send_to(buffer(SNTPQueryData, 48),
                *sel, std::bind (&SNTPClientImp::onSend, this,
                    std::placeholders::_1,
                        std::placeholders::_2));
        }
    }

    void onSend (error_code const& ec, std::size_t)
    {
        std::cout << "ec = " << ec << std::endl;
        if (ec == boost::asio::error::operation_aborted)
            return;

        if (ec)
        {
            std::cout <<
                "SNTPClock::onSend: " << ec.message();
            return;
        }
        using namespace boost::asio;
        //socket_.open (ip::udp::v4 ());
        socket_.async_receive_from (buffer (buf_, 256), //256
            ep_, std::bind(
                &SNTPClientImp::onRead, this,
                    std::placeholders::_1,
                        std::placeholders::_2));
        //timer_.expires_from_now(std::chrono::seconds(10));
        //timer_.async_wait(std::bind(
        //    &SNTPClientImp::onTimer, this,
        //        std::placeholders::_1));
        std::cout << "Data sended! waiting received massage." << std::endl;
    }

    void processReply ()
    {
        using namespace std::chrono;
        assert (buf_.size () >= 48);
        std::uint32_t* recvBuffer = reinterpret_cast<std::uint32_t*> (&buf_.front ());

        unsigned info = ntohl (recvBuffer[NTP_OFF_INFO]);
        auto timev = seconds{ntohl(recvBuffer[NTP_OFF_RECVTS_INT])};
        unsigned stratum = (info >> 16) & 0xff;

        if ((info >> 30) == 3)
        {
            std::cout <<
                "SNTP: Alarm condition " << ep_;
            return;
        }

        if ((stratum == 0) || (stratum > 14))
        {
            std::cout <<
                "SNTP: Unreasonable stratum (" << stratum << ") from " << ep_;
            return;
        }

        using namespace std::chrono;
        auto now = time_point_cast<seconds>(clock_type::now());
        timev -= now.time_since_epoch();
        timev -= NTP_UNIX_OFFSET;

        // add offset to list, replacing oldest one if appropriate
        offsets_.push_back (timev);

        if (offsets_.size () >= NTP_SAMPLE_WINDOW)
            offsets_.pop_front ();

        lastUpdate_ = now;

        // select median time
        auto offsetList = offsets_;
        std::sort(offsetList.begin(), offsetList.end());
        auto j = offsetList.size ();
        auto it = std::next(offsetList.begin (), j/2);
        offset_ = *it;

        if ((j % 2) == 0)
            offset_ = (offset_ + (*--it)) / 2;

        // debounce: small corrections likely
        //           do more harm than good
        if ((offset_ == -1s) || (offset_ == 1s))
            offset_ = 0s;

        if (timev != 0s || offset_ != 0s)
        {
            std::cout << "SNTP: Offset is " << timev.count() <<
                ", new system offset is " << offset_.count();
        }
    }
};

//------------------------------------------------------------------------------

std::unique_ptr<SNTPClock>
make_SNTPClock ()
{
    return std::make_unique<SNTPClientImp>();
}

} // ripple


int main(int argc, char* argv[])
{
	std::vector<std::string> serv{"sgp.ntp.org.cn","cn.ntp.org.cn","hk.ntp.org.cn","us.ntp.org.cn","edu.ntp.org.cn"};
	auto clock_ = ripple::make_SNTPClock();
	clock_->doRun(serv);
	clock_->run();
    system("pause");
	std::cout << "Exit: main()" << std::endl;
	return 0;
}
