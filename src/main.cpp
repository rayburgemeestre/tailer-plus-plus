#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <unordered_map>

#include <inotify-cpp/FileSystemAdapter.h>
#include <inotify-cpp/NotifierBuilder.h>

#include "beej.h"
#include "logger.hpp"
#include "program_options.h"
#include "server.h"
#include "util.h"

using namespace inotify;

int main(int argc, char** argv) {
  program_options options(argc, argv);

  std::unique_ptr<server> serv = nullptr;
  std::unique_ptr<beej::client> client = nullptr;

  if (argc > 2) {
    serv = std::make_unique<server>(options);
    client = std::make_unique<beej::client>(options.server_host, std::stoi(options.server_port));
    client->connect();
  }

  std::unordered_map<std::string, size_t> files;
  std::unordered_map<std::string, std::string> buffers;
  std::mutex mut;

  const auto hostname = get_hostname();
  std::cout << "Our Host Name: " << hostname << std::endl;

  logger log([&](const std::string& line) {
    static std::stringstream ss;
    if (client) {
      auto offset = (serv->our_start_time() - serv->our_time_now());
      auto time = serv->ntp_start_time() - offset;
      time_t epoch_seconds = static_cast<time_t>(time / 1000);
      std::tm* tm2 = std::localtime(&epoch_seconds);
      int millis = static_cast<size_t>(time) % 1000;
      ss << std::put_time(tm2, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << millis;
      ss << " [" << hostname << "] " << line;
      client->send(ss.str());
    } else {
      std::cout << line << std::flush;
    }
    ss.str("");
    ss.clear();
  });

  auto filepath = std::string(argv[1]);
  const auto s = std::filesystem::status(filepath);
  if (std::filesystem::is_directory(s)) {
    // Read all files in the directory and record all the file sizes
    using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
    for (const auto& entry : recursive_directory_iterator(filepath)) {
      if (!entry.is_regular_file()) continue;
      files[entry.path().string()] = entry.file_size();
    }
  } else if (std::filesystem::is_regular_file(s)) {
    // Read file in and record the file sizes
    files[filepath] = std::filesystem::file_size(std::filesystem::path(filepath));
  } else if (filepath == "-" || filepath == "/dev/stdin") {
    // special case where we cannot use inotify
    log << "special case for stdin" << std::endl;
    std::string line;
    while (std::getline(std::cin, line)) {
      // TODO: support custom label here...
      log << "*stdin*: " << line << std::endl;
    }
    exit(0);
  } else {
    options.usage(argv);
  }

  log << "Initialized " << files.size() << " files." << std::endl;

  inotifypp::filesystem::path path(filepath);

  auto handleNotification = [&](Notification notification) {
    auto full_file_path = notification.path.string();
    auto filename = notification.path.filename().string();
    try {
      if (notification.event == inotify::Event::modify || notification.event == inotify::Event::create) {
        size_t current_size =
            std::filesystem::file_size(notification.path);  // might throw in case file was deleted in the meantime

        // Lookup last known filesize for file (otherwise assume zero)
        std::unique_lock lock(mut);
        if (files.find(full_file_path) == files.end()) {
          files[full_file_path] = 0;
        }

        // Seek to the last known position
        std::ifstream fd(full_file_path);
        if (current_size > files[full_file_path]) {
          fd.seekg(files[full_file_path]);
        }

        // Assume the file was truncated if the current file size is less than previously known (reset to beginning)
        if (current_size < files[full_file_path]) {
          files[full_file_path] = 0;
          fd.seekg(0);
          log << filename << ": *** truncated ***" << std::endl;
        }

        // Keep reading as long as we're still behind the current files last position
        while (true) {
          size_t read = current_size - files[full_file_path];
          if (read <= 0) {
            break;
          }
          std::vector<char> buffer;
          buffer.resize(read);
          size_t r = fd.readsome(buffer.data(), read);

          // Add read chunk of data to the buffer (note that we might read blocks of multiple- and/or partial lines)
          buffers[full_file_path].insert(buffers[full_file_path].end(), buffer.begin(), buffer.end());

          // Advance our pointer to what we've read just know
          files[full_file_path] += r;

          // Keep printing lines as long as we have lines in the buffer.
          while (true) {
            auto pos = buffers[full_file_path].find("\n");
            if (pos == std::string::npos) {
              break;
            }
            // Print prefixed with the basename of the file
            log << filename << ": " << buffers[full_file_path].substr(0, pos) << std::endl;
            buffers[full_file_path] = buffers[full_file_path].substr(pos + 1);
          }
        }
      } else if (notification.event == inotify::Event::remove) {
        files.erase(full_file_path);
        buffers.erase(full_file_path);
      } else {
        log << "Event " << notification.event << " on " << notification.path << " at "
            << notification.time.time_since_epoch().count() << " was triggered." << std::endl;
      }
    } catch (std::filesystem::filesystem_error& err) {
      std::cerr << err.what() << std::endl;
    }
  };

  auto handleUnexpectedNotification = [](Notification notification) {};
  auto events = {Event::create, Event::modify, Event::remove, Event::move};
  auto notifier = BuildNotifier()
                      .watchPathRecursively(path)
                      .onEvents(events, handleNotification)
                      .onUnexpectedEvent(handleUnexpectedNotification);

  log << "Listening with inotify.. Press Control+C to stop the process." << std::endl << "---" << std::endl;

  notifier.run();

  return 0;
}
