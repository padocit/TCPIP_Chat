#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <memory>
#include <thread>

using boost::asio::ip::tcp;

class Client : public std::enable_shared_from_this<Client>
{
public:
    Client(boost::asio::io_context& io_context, const std::string& host, const std::string& port)
        : socket_(io_context), resolver_(io_context)
    {
        auto endpoints = resolver_.resolve(host, port);
        boost::asio::async_connect(socket_, endpoints,
            [this](boost::system::error_code ec, tcp::endpoint) {
                if (!ec)
                {
                    std::cout << "Connected to server!" << std::endl;
                    Receive();
                }
                else
                {
                    std::cerr << "Connection failed: " << ec.message() << std::endl;
                }
            });
    }

    void Send(std::string msg)
    {
        auto self(shared_from_this());
        // msg를 shared_ptr로 감싸서 람다로 전달
        auto msg_ptr = std::make_shared<std::string>(std::move(msg));

        boost::asio::async_write(socket_, boost::asio::buffer(*msg_ptr),
            [this, self, msg_ptr](boost::system::error_code ec, std::size_t /*length*/) {                if (ec)
                {
                    std::cerr << "Send failed: " << ec.message() << std::endl;
                }
            });
    }

private:
    void Receive()
    {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec && length > 0)
                {
                    std::cout << "\nServer: " << std::string(data_.data(), length) << std::endl;

                    // Clear the input line prompt and redisplay it
                    std::cout << "Me: ";
                    std::cout.flush();

                    Receive();
                }
                else if (ec)
                {
                    std::cerr << "Receive failed: " << ec.message() << std::endl;
                }
            });
    }

    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;
    std::array<char, 1024> data_;
};

int main()
{
	try
	{
		boost::asio::io_context io_context;

		// client
		auto client = std::make_shared<Client>(io_context, "127.0.0.1", "8080");

		// thread
		std::thread input_thread([&client]() {
			while (true) 
			{
				std::cout << "Me: ";
				std::string msg;

				std::getline(std::cin, msg);

				if (msg == "!q" || msg == "quit" || msg == "exit") 
				{
					std::cout << "Exiting..." << std::endl;
					break;
				}

				client->Send(msg);
			}
		});

		io_context.run();

		input_thread.join();
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}

	return 0;
}