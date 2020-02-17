//
// blocking_udp_echo_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp;
static uint8_t SNTPQueryData[48] =
{ 0x1B, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

enum { max_length = 1024 };
boost::asio::io_service io_service;
udp::endpoint endpoint;
// 端口0作为动态端口，委托操作系统来找到一个实际可用的端口
udp::socket s(io_service, udp::endpoint(udp::v4(), 0));

void test()
{
	//udp::socket s(io_service, udp::endpoint(udp::v4(), 0));
	std::cout << "Enter message: ";
    char request[max_length];
    std::cin.getline(request, max_length);
    size_t request_length = std::strlen(request);
    //s.send_to(boost::asio::buffer(request, request_length), endpoint);
    s.send_to(boost::asio::buffer(SNTPQueryData, 48),endpoint);

    char reply[max_length];
    udp::endpoint sender_endpoint;
    s.async_receive_from(
        boost::asio::buffer(reply, max_length), sender_endpoint,
        [&](boost::system::error_code ec, std::size_t bytes_recvd)
        {
          if (!ec && bytes_recvd > 0)
          {
            std::cout << std::string(request) << "Reply is: ";
    		    std::cout.write(reply, bytes_recvd);
    		    std::cout << "\n";
    		    test();
          }
        });
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: blocking_udp_echo_client <host> <port>\n";
      return 1;
    }

    
    
    udp::resolver resolver(io_service);
    endpoint = *resolver.resolve({udp::v4(), argv[1], argv[2]});

    test();
    /*
    size_t reply_length = s.receive_from(
        boost::asio::buffer(reply, max_length), sender_endpoint);
    std::cout << "Reply is: ";
    std::cout.write(reply, reply_length);
    std::cout << "\n";
    */
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}