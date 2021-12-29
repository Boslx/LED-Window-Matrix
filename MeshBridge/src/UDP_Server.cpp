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
        unsigned const bufferdFramesCount = 25;

        if (!ec && (bytes_transferred > 0)) {
          std::string totalMessage = std::string(
              &recv_buffer_[0], &recv_buffer_[0] + bytes_transferred);

          uint32_t timestamp = mesh_.getNodeTime() + 2000000;
          std::cout << "Timestamp " << timestamp << std::endl;
          std::string timestampString = getTimestampAsString(timestamp);

          unsigned lengthSingleFrame = bytes_transferred / bufferdFramesCount;
          unsigned realPixelRowGroupLength = pixelRowGroupLength_ * 3;

          std::vector<std::string> frames =
              splitIntoFrames(totalMessage, lengthSingleFrame);

          unsigned currentDevice = 0;
          for (unsigned i = 0; i < lengthSingleFrame;
               i += realPixelRowGroupLength) {
            std::string nodeMessage = timestampString;
            for (auto frame : frames) {
              nodeMessage.append(frame.substr(i, realPixelRowGroupLength));
            }

            auto targetDevice = channelDestMapping_[currentDevice];
            mesh_.sendSingle(targetDevice,
                             macaron::Base64::Encode(nodeMessage));

            std::cout << "Sending message to " << targetDevice << " with length " << nodeMessage.length() << std::endl;
            ++currentDevice;
          }
        } else
          throw ec;  // TODO
        start_receive();
      });
}
std::vector<std::string> UDP_Server::splitIntoFrames(
    const std::string& totalMessage, unsigned int lengthSingleFrame) const {
  std::vector<std::string> frames;
  for (unsigned i = 0; i < totalMessage.length(); i += lengthSingleFrame) {
    frames.push_back(totalMessage.substr(i, lengthSingleFrame));
  }
  return frames;
}
std::string UDP_Server::getTimestampAsString(uint32_t timestamp) const {
  std::string timestampString;

  timestampString.push_back(timestamp >> 24);
  timestampString.push_back(timestamp >> 16);
  timestampString.push_back(timestamp >> 8);
  timestampString.push_back(timestamp);
  return timestampString;
}
