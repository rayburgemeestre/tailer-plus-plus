#pragma once

#include <chrono>
#include <string>

class program_options;

class server {
public:
  server(const program_options& options);

  double start_time();
  double current_time();

private:
  double ntptime;
  std::chrono::high_resolution_clock::time_point start;
};