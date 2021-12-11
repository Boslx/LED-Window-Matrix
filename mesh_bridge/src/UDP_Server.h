//
// Created by leonhard on 21.11.21.
//
#ifndef PAINLESSMESHBOOST_UDP_SERVER_H
#define PAINLESSMESHBOOST_UDP_SERVER_H


#include <boost/asio/io_service.hpp>
#include "painlessmesh/protocol.hpp"
#include "painlessmesh/mesh.hpp"
#include "painlessMesh.h"
#include "painlessMeshConnection.h"
#include "plugin/performance.hpp"

using boost::asio::ip::udp;

class UDP_Server {
 public:
  UDP_Server(uint pixelRowGroupLength, boost::asio::io_service &io_service, painlessMesh& mesh, std::map<uint8_t, uint32_t>& channelDestMapping);

 private:
  udp::socket socket_;
  painlessMesh& mesh_;
  char recv_buffer_[1024]{};
  uint pixelRowGroupLength_;
  udp::endpoint remote_endpoint_;
  std::map<uint8_t, uint32_t>& channelDestMapping_;
  void start_receive();
};

#endif  // PAINLESSMESHBOOST_UDP_SERVER_H
