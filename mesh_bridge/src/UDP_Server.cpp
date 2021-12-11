//
// Created by leonhard on 21.11.21.
//

#include "UDP_Server.h"
UDP_Server::UDP_Server(uint pixelRowGroupLength,
                       boost::asio::io_service& io_service, painlessMesh& mesh,
                       std::map<uint8_t, uint32_t>& channelDestMapping)
    : pixelRowGroupLength_(pixelRowGroupLength),
      socket_(io_service, udp::endpoint(udp::v4(), 7890)),
      mesh_(mesh),
      channelDestMapping_(channelDestMapping) {
  start_receive();
}
void UDP_Server::start_receive() {
  socket_.async_receive_from(
      boost::asio::buffer(recv_buffer_), remote_endpoint_,
      [this](boost::system::error_code ec, std::size_t bytes_transferred) {
        if (!ec && (bytes_transferred > 0)) {
          std::string totalMessage = std::string(
              &recv_buffer_[0], &recv_buffer_[0] + bytes_transferred);

          unsigned currentDevice = 0;
          uint deviceMessageLength =
              pixelRowGroupLength_ * 3;  // for rgb we need 3 byte
          for (unsigned i = 0; i < totalMessage.length();
               i += deviceMessageLength) {
            auto targetDevice = channelDestMapping_[currentDevice];
            auto deviceMessage = totalMessage.substr(i, deviceMessageLength);

            mesh_.sendSingle(targetDevice, deviceMessage);
            ++currentDevice;
          }
        } else
          throw ec;  // TODO
        start_receive();
      });
}
