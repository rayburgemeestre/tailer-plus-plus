#include "program_options.h"

#include <iostream>

#include "util.h"

program_options::program_options(int argc, char** argv) {
  if (argc <= 1) {
    usage(argv);
  }
  // defaults
  std::string central_server = "localhost:10000";
  std::string timeserver = "time.google.com:123";

  if (argc > 2) {
    central_server = std::string(argv[2]);
    auto pos = central_server.find(":");
    if (pos == std::string::npos) {
      usage(argv);
    }
    server_host = central_server.substr(0, pos);
    server_port = central_server.substr(pos + 1);
    std::cout << "Log Server: " << server_host << ", " << server_port << std::endl;
  }
  if (argc > 3) {
    timeserver = std::string(argv[3]);
  }
  auto pos = timeserver.find(":");
  if (pos == std::string::npos) {
    usage(argv);
  }
  timeserver_host = timeserver.substr(0, pos);
  timeserver_port = timeserver.substr(pos + 1);
  timeserver_ip = get_ip(timeserver_host);
  std::cout << "Time Server host: " << timeserver_host << ", " << timeserver_ip << std::endl;
  std::cout << "Time Server port: " << timeserver_port << std::endl;
}

void program_options::usage(char** argv) {
  std::cout << "Usage: " << argv[0] << " <directory | file> [ <server:port> [ <timeserver> ] ]" << std::endl
            << "  e.g. " << argv[0] << " /var/log" << std::endl
            << "  e.g. " << argv[0] << " /var/log 127.0.0.1:3456" << std::endl
            << "  e.g. " << argv[0] << " /var/log 127.0.0.1:3456 time.xyz.com:123" << std::endl
            << "\n"
            << "\n"
            << "When providing a server, an NTP time is fetched so we can\n"
            << "properly timestamp our logs as we send them to the server.\n"
            << "Default timeserver used is: time.google.com:123.\n"
            << std::endl;
  exit(1);
}