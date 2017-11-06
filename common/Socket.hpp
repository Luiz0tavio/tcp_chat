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

		enum EVENT_TYPE {
			KEYBOARD 	= 0x1 << 0,
			RECEIVE 	= 0x1 << 1,
			ACCEPT 		= 0x1 << 2
		};

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

		void tcpSelect(int m_socket_, std::vector<int>c_sockets_ = {}, const std::function<void(int, int)>& handler = nullptr)
		{

			fd_set master = {};
			fd_set rfds = {};

			FD_ZERO(&master);
			FD_ZERO(&rfds);

			int nfds = m_socket_;
			FD_SET(m_socket_, &master);
			FD_SET(STDIN_FILENO, &master);
			for (auto c_socket : c_sockets_) {
				nfds = c_socket > nfds ? c_socket : nfds;
				FD_SET(c_socket, &master);
			}

			struct timeval tv = {};
			tv.tv_sec = 5;
			tv.tv_usec = 0;

			while (true) {
				rfds = master;
				int retval = select(nfds+1, &rfds, nullptr, nullptr, &tv);
				if (retval > 0) {
					if (FD_ISSET(STDIN_FILENO, &rfds)) {
						handler(EVENT_TYPE::KEYBOARD, m_socket_);
					}
					if (FD_ISSET(m_socket_, &rfds)) {
						handler(EVENT_TYPE::ACCEPT | EVENT_TYPE::RECEIVE, m_socket_);
					}
					for (auto c_socket : c_sockets_) {
						if (FD_ISSET(c_socket, &rfds)) {
							handler(EVENT_TYPE::RECEIVE, c_socket);
						}
					}
				}
			}
		}

        void tcpSend(int socket_, Common::Protocol* protocol_)
        {
			protocol_->headerToNetworkOrder();
			protocol_->msgToNetworkOrder();

			size_t send_size = 0;
			if (protocol_->hasMsg()) {
				send_size = sizeof(header_str) + sizeof(msg_str);
				std::unique_ptr<char> buffer ((char *)malloc(send_size));
				memcpy(buffer.get(), protocol_->getHeader(), sizeof(header_str));
				memcpy(buffer.get()+sizeof(header_str), protocol_->getMsg(), sizeof(msg_str));

				send(socket_, buffer.get(), send_size, 0);
			} else {
				send_size = sizeof(header_str);
				std::unique_ptr<char> buffer((char *) malloc(send_size));
				memcpy(buffer.get(), protocol_->getHeader(), send_size);

				send(socket_, buffer.get(), send_size, 0);
			}
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
