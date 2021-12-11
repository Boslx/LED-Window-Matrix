#include <algorithm>
#include <boost/algorithm/hex.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <iterator>

#include "Arduino.h"

WiFiClass WiFi;
ESPClass ESP;

#include "UDP_Server.h"
#include "painlessMesh.h"
#include "painlessMeshConnection.h"
#include "painlessmesh/mesh.hpp"
#include "painlessmesh/protocol.hpp"
#include "plugin/performance.hpp"

painlessmesh::logger::LogClass Log;

#undef F
#include <boost/date_time.hpp>
#include <boost/program_options.hpp>
#define F(string_literal) string_literal
namespace po = boost::program_options;

#include <iostream>
#include <iterator>
#include <limits>
#include <random>

#define OTA_PART_SIZE 1024
#include "ota.hpp"

template <class T>
// bool contains(T &v, T::value_type const value) {
bool contains(T& v, std::string const value) {
  return std::find(v.begin(), v.end(), value) != v.end();
}

std::string timeToString() {
  boost::posix_time::ptime timeLocal =
      boost::posix_time::second_clock::local_time();
  return to_iso_extended_string(timeLocal);
}

// Will be used to obtain a seed for the random number engine
static std::random_device rd;
static std::mt19937 gen(rd());

uint32_t runif(uint32_t from, uint32_t to) {
  std::uniform_int_distribution<uint32_t> distribution(from, to);
  return distribution(gen);
}

std::map<uint8_t, uint32_t> getChannelDestMapping(std::string filename) {
  std::ifstream inFile(filename, std::ios::in);
  if (!inFile.is_open())
  {
    std::cerr << "Can't open the file" << std::endl;
    throw "Not able to";
  }
  std::string lineStr;
  std::map<uint8_t, uint32_t> channelDestMapping;
  while (getline(inFile,lineStr))
  {
    //  Split string
    int index = lineStr.find(",");
    uint8_t channel = std::stoul(lineStr.substr(0, index));
    uint32_t destId = std::stoul(lineStr.substr(index+1, lineStr.size()-1));
    //  Save to map
    channelDestMapping[channel] = destId;
  }

  return channelDestMapping;
}

int main(int ac, char* av[]) {
  using namespace painlessmesh;
  size_t port = 5555;
  std::string ip = "";
  std::vector<std::string> logLevel;
  size_t nodeId = runif(0, std::numeric_limits<uint32_t>::max());
  std::string otaDir;
  double performance = 2.0;

  try {
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "Produce this help message")(
        "nodeid,n", po::value<size_t>(&nodeId),
        "Set nodeID, otherwise set to a random value")(
        "port,p", po::value<size_t>(&port), "The mesh port (default is 5555)")(
        "server,s",
        "Listen to incoming node connections. This is the default, unless "
        "--client "
        "is specified. Specify both if you want to both listen for incoming "
        "connections and try to connect to a specific node.")(
        "client,c", po::value<std::string>(&ip),
        "Connect to another node as a client. You need to provide the ip "
        "address of the node.")(
        "log,l", po::value<std::vector<std::string>>(&logLevel),
        "Only log given events to the console. By default all events are "
        "logged, this allows you to filter which ones to log. Events currently "
        "logged are: receive, connect, disconnect, change, offset and delay. "
        "This option can be specified multiple times to log multiple types of "
        "events.")("ota-dir,d", po::value<std::string>(&otaDir),
                   "Watch given folder for new firmware files.")(
        "performance", po::value<double>(&performance)->implicit_value(2.0),
        "Enable performance monitoring. Optional value is frequency (per "
        "second) to send performance monitoring packages. Default is every 2 "
        "seconds.");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 0;
    }

    Scheduler scheduler;
    boost::asio::io_service mesh_service;
    // boost::asio::io_service udp_server_service;
    painlessMesh mesh;
    Log.setLogLevel(ERROR);
    mesh.init(&scheduler, nodeId);
    std::shared_ptr<AsyncServer> pServer;
    if (vm.count("server") || !vm.count("client")) {
      pServer = std::make_shared<AsyncServer>(mesh_service, port);
      painlessmesh::tcp::initServer<MeshConnection, painlessMesh>(*pServer,
                                                                  mesh);
    }

    if (vm.count("client")) {
      auto pClient = new AsyncClient(mesh_service);
      painlessmesh::tcp::connect<MeshConnection, painlessMesh>(
          (*pClient), boost::asio::ip::address::from_string(ip), port, mesh);
    }

    if (logLevel.size() == 0 || contains(logLevel, "receive")) {
      mesh.onReceive([&mesh](uint32_t nodeId, std::string& msg) {
        std::cout << "{\"event\":\"receive\",\"nodeTime\":"
                  << mesh.getNodeTime() << ",\"time\":\"" << timeToString()
                  << "\""
                  << ",\"nodeId\":" << nodeId << ",\"msg\":\"" << msg << "\"}"
                  << std::endl;
      });
    }
    if (logLevel.size() == 0 || contains(logLevel, "connect")) {
      mesh.onNewConnection([&mesh](uint32_t nodeId) {
        std::cout << "{\"event\":\"connect\",\"nodeTime\":"
                  << mesh.getNodeTime() << ",\"time\":\"" << timeToString()
                  << "\""
                  << ",\"nodeId\":" << nodeId
                  << ", \"layout\":" << mesh.asNodeTree().toString() << "}"
                  << std::endl;
      });
    }

    if (logLevel.size() == 0 || contains(logLevel, "disconnect")) {
      mesh.onDroppedConnection([&mesh](uint32_t nodeId) {
        std::cout << "{\"event\":\"disconnect\",\"nodeTime\":"
                  << mesh.getNodeTime() << ",\"time\":\"" << timeToString()
                  << "\""
                  << ",\"nodeId\":" << nodeId
                  << ", \"layout\":" << mesh.asNodeTree().toString() << "}"
                  << std::endl;
      });
    }

    if (logLevel.size() == 0 || contains(logLevel, "change")) {
      mesh.onChangedConnections([&mesh]() {
        std::cout << "{\"event\":\"change\",\"nodeTime\":" << mesh.getNodeTime()
                  << ",\"time\":\"" << timeToString() << "\""
                  << ", \"layout\":" << mesh.asNodeTree().toString() << "}"
                  << std::endl;
      });
    }

    if (logLevel.size() == 0 || contains(logLevel, "offset")) {
      mesh.onNodeTimeAdjusted([&mesh](int32_t offset) {
        std::cout << "{\"event\":\"offset\",\"nodeTime\":" << mesh.getNodeTime()
                  << ",\"time\":\"" << timeToString() << "\""
                  << ",\"offset\":" << offset << "}" << std::endl;
      });
    }

    if (logLevel.size() == 0 || contains(logLevel, "delay")) {
      mesh.onNodeDelayReceived([&mesh](uint32_t nodeId, int32_t delay) {
        std::cout << "{\"event\":\"delay\",\"nodeTime\":" << mesh.getNodeTime()
                  << ",\"time\":\"" << timeToString() << "\""
                  << ",\"nodeId\":" << nodeId << ",\"delay\":" << delay << "}"
                  << std::endl;
      });
    }

    if (vm.count("performance")) {
      plugin::performance::begin(mesh, performance);
    }

    auto thing = getChannelDestMapping("ChannelDestMapping.csv");

    do {
      usleep(1000);
      mesh.update();
      mesh_service.poll();
      std::cout << "Waiting for nodes..." << std::endl;
    } while (mesh.getNodeList().empty());

    auto nodeList = mesh.getNodeList();
    UDP_Server udp_Server(mesh_service, mesh, thing);
    while (true) {
      usleep(1000);  // Tweak this for acceptable cpu usage
      mesh.update();
      mesh_service.poll();
    }
  } catch (boost::system::error_code e) {
    std::cout << e.message() << std::endl;
  }

  return 0;
}
