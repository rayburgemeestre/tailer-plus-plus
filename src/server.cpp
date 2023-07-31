#include "server.h"
#include "program_options.h"

#include "ntp_client.hpp"

#include <iostream>

server::server(const program_options& options) {
  NTPClient client{options.timeserver_ip, (size_t)std::stoi(options.timeserver_port)};
  ntptime = client.request_time();

  start = std::chrono::high_resolution_clock::now();
  std::cout << "NTP Time: " << std::fixed << ntptime << std::endl;

  auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(start);
  auto epoch = now_ms.time_since_epoch();
  start_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();

  auto epoch_server_ms = ntptime;  // client.request_time();

  if (0 == epoch_server_ms) exit(1);

  // The function ctime receives the timestamps in seconds.
  time_t epoch_server = (uint32_t)(epoch_server_ms / 1000);

  std::cout << "Server time: " << ctime(&epoch_server);
  std::cout << "Timestamp server: " << (uint32_t)epoch_server << "\n\n";

  time_t local_time;
  local_time = time(0);

  std::cout << "System time is " << (epoch_server - local_time) << " seconds off\n";
}

double server::ntp_start_time() {
  return ntptime;
}

double server::our_start_time() {
  return start_epoch;
}

double server::our_time_now() {
  auto start = std::chrono::high_resolution_clock::now();
  auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(start);
  auto epoch = now_ms.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
}

double server::current_time() {
  auto end_time = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(end_time - start).count() / 1000.;
}
