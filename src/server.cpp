#include "server.h"
#include "program_options.h"

#include "client.hpp"

#include <iostream>

server::server(const program_options& options) {
  NTPClient client{options.timeserver_ip, (size_t)std::stoi(options.timeserver_port)};
  ntptime = client.request_time();

  start = std::chrono::high_resolution_clock::now();
  std::cout << "NTP Time: " << std::fixed << ntptime << std::endl;
}

double server::start_time() {
  return ntptime;
}

double server::current_time() {
  auto end_time = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(end_time - start).count() / 1000.;
}