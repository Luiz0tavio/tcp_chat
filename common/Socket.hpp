//
// Created by tracksale on 10/31/17.
//

#ifndef TCP_CHATsocket__HPP
#define TCP_CHATsocket__HPP

#include <sys/socket.h>
#include <stdexcept>
#include <vector>
#include <functional>
#include "Protocol.hpp"

#define LOCAL_HOST "127.0.0.1"

namespace Common {

    class Socket {

        int socket_;

    public:


    protected:

        Socket()
        {
            socket_ = socket(AF_INET, SOCK_STREAM, 0);
            if (socket_ < 0) {
                throw std::runtime_error("ERROR opening socket");
            }

        }

        int getSocket()
        {
            return socket_;
        }

		void tcpSelect(int socket_, bool watch_stdin=false, const std::function<void(Protocol*)>& handler = nullptr)
		{
			char buf[35];

			fd_set master = {};
			fd_set rfds = {};

			FD_ZERO(&master);
			FD_ZERO(&rfds);
			FD_SET(socket_, &master);
			if (watch_stdin) {
				FD_SET(STDIN_FILENO, &master);
			}

			struct timeval tv = {};
			tv.tv_sec = 5;
			tv.tv_usec = 0;

			while(true) {
				rfds = master;
				int retval = select(socket_+1, &rfds, nullptr, nullptr, &tv);
				if (retval > 0) {
					if (FD_ISSET(STDIN_FILENO, &rfds)) {
						if (fgets(buf, 35, stdin)) {
							std::cout << "Eu: " << buf << std::endl;
							// @TODO handle keyboard
							// handler(buf);
						}
					}
					if (FD_ISSET(socket_, &rfds)) {
						Common::Protocol* protocol_ = this->receive(socket_);
						handler(protocol_);
					}
				} else {
					std::cout << "retval failed: " << retval << std::endl;
					std::cout << "socket disconnected: " << socket_ << std::endl;
					close(socket_);
					break;
				}
			}
		}

        void tcpSend(int socket_, Common::Protocol* protocol_)
        {
            size_t send_size = protocol_->hasMsg() ? sizeof(header_str) + sizeof(msg_str) : sizeof(header_str);
            send(socket_, protocol_, send_size, 0);
        }

		Common::Protocol* receive (int socket)
        {
            size_t buffer_size = sizeof(Common::header_str) + sizeof(Common::msg_str);
            std::unique_ptr<char> buffer ((char *)malloc(buffer_size));
            ssize_t recv_size = recv(socket, buffer.get(), buffer_size, 0);

            if (recv_size > 0) {
                return Common::Protocol::getProtocolFromBuffer(buffer.get());
            } else {
                throw "asd";
            }
        }

    };
}

#endif //TCP_CHATsocket__HPP
