/*
* Boost asio
* https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/examples.html
* https://theboostcpplibraries.com/boost.asio-io-services-and-io-objects
* 
* Boost 1.66 이후: io_service -> io_context
*/

#include <iostream>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <thread>
#include <future>
#include <memory>
#include <algorithm>

static unsigned gSessionCount = 0;

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::ip::tcp::socket socket, std::vector<std::shared_ptr<Session>>& sessions, unsigned id)
		: socket_(std::move(socket)), sessions_(sessions), id_(id)
	{}

	~Session()
	{
		std::cout << "Session " << id_ << " destroyed." << std::endl;
	}

	void start() 
	{ 
		sessions_.push_back(shared_from_this());
		do_read(); 
	}


private:
	void do_read() 
	{
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(data_),
			[this, self](boost::system::error_code ec, std::size_t length) {
				if (!ec && length > 0)
				{
					auto received_msg = std::make_shared<std::string>(std::string(data_.data(), length));
					std::cout << "Client "<<id_ << ": " << *received_msg << std::endl;
					
					broadcast(received_msg);

					do_read();
				}
				else 
				{
					// 에러 처리 및 세션 정리
					if (ec == boost::asio::error::eof) {
						std::cout << "Client " << id_ << " disconnected." << std::endl;
					}
					else {
						std::cerr << "Read error (Client " << id_ << "): " << ec.message() << std::endl;
					}
					close_session();
				}
			});
	}

	void broadcast(const std::shared_ptr<std::string> msg)
	{
		for (auto& session : sessions_)
		{
			if (session.get() != this) // 본인 제외
			{
				session->deliver(msg);
			}
		}
	}

	void deliver(const std::shared_ptr<std::string> msg) {
		auto self(shared_from_this());
		boost::asio::async_write(socket_, boost::asio::buffer(*msg), 
			[this, self, msg](boost::system::error_code ec, std::size_t /*length*/) {
				if (ec)
				{
					std::cerr << "Write error (Client " << id_ << "): " << ec.message() << std::endl;
					// 에러 시 세션 제거 
					close_session();
				}
			});
	}

	void close_session()
	{
		// 소켓 닫기
		boost::system::error_code ignored_ec;
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		socket_.close(ignored_ec);

		// sessions_에서 제거
		auto self = shared_from_this();
		sessions_.erase(std::remove(sessions_.begin(), sessions_.end(), self), sessions_.end());
		// self 포인터가 sessions_에서 제거되면 참조 카운트 감소 -> 소멸
	}

	boost::asio::ip::tcp::socket socket_;
	std::array<char, 1024> data_;
	std::vector<std::shared_ptr<Session>>& sessions_;
	unsigned id_;
};

void start_accept(boost::asio::ip::tcp::acceptor& acceptor, std::vector<std::shared_ptr<Session>>& sessions)
{
	acceptor.async_accept(
		[&acceptor, &sessions](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
			if (!ec)
			{
				gSessionCount += 1;
				std::cout << "Client connected: " << socket.remote_endpoint() << std::endl;

				std::make_shared<Session>(std::move(socket), sessions, gSessionCount)->start();
			}

			start_accept(acceptor, sessions);
		});
}

int main()
{
	try
	{
		boost::asio::io_context io_context;

		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 8080);
		boost::asio::ip::tcp::acceptor acceptor(io_context, endpoint);

		std::vector<std::shared_ptr<Session>> sessions;

		std::cout << "Server is running on port: " << endpoint.port() << std::endl;

		start_accept(acceptor, sessions);

		io_context.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}

/*
TODO 1: 동시 채팅 시스템 (서버-클라 구조)
TODO 2: 여러 명의 클라이언트 받기
*/