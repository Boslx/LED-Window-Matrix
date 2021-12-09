//
// Created by leonhard on 21.11.21.
//

#include "UDP_Server.h"
UDP_Server::UDP_Server(boost::asio::io_service& io_service, painlessMesh& mesh,
                       std::list<uint32_t>& nodeList)
    : socket_(io_service, udp::endpoint(udp::v4(), 7890)),
      mesh_(mesh),
      nodeList_(nodeList) {
  start_receive();
}
void UDP_Server::start_receive() {
  socket_.async_receive_from(
      boost::asio::buffer(recv_buffer_), remote_endpoint_,
      [this](boost::system::error_code ec, std::size_t bytes_transferred) {
        if (!ec && (bytes_transferred > 0)) {
          std::string message = std::string(
              &recv_buffer_[0], &recv_buffer_[0] + bytes_transferred);
          for (auto const& i : nodeList_) {
            mesh_.sendSingle(i, message);
          }
        } else
          throw ec;  // TODO
        start_receive();
      });
}
