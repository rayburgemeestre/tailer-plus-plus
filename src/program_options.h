#pragma once

#include <string>

class program_options {
public:
  program_options(int argc, char** argv);

  void usage(char** argv);

  std::string server_host;
  std::string server_port;
  std::string timeserver_host;
  std::string timeserver_port;
  std::string timeserver_ip;
};
