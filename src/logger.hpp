#include <fstream>
#include <memory>
#include <mutex>

using std::chrono::system_clock;

#include <chrono>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

static std::recursive_mutex logger_mut__;

class metrics;

class logger {
  std::stringstream ss_;
  using callback_t = std::function<void(const std::string &)>;
  callback_t callback_ = [](const std::string &) {};

public:
  explicit logger(callback_t cb) : callback_(cb), _holder(std::make_shared<holder>(ss_, callback_)) {}
  explicit logger(std::stringstream &ss, callback_t &cb) : _holder(std::make_shared<holder>(ss, cb)) {}

  friend logger operator<<(const logger &l, std::ostream &(*fun)(std::ostream &)) {
    l._holder->_ss << std::endl;
    auto s = l._holder->_ss.str();
    l._holder->callback_(s);
    l._holder->_ss.str("");
    l._holder->_ss.clear();
    return logger(l._holder->_ss, l._holder->callback_);
  }

  template <class T>
  friend logger operator<<(const logger &l, const T &t) {
    l._holder->_ss << t;
    return logger(l._holder->_ss, l._holder->callback_);
  }

private:
  struct holder {
    explicit holder(std::stringstream &ss, callback_t &cb) : _ss(ss), callback_(cb), _lock(logger_mut__) {}
    std::stringstream &_ss;
    callback_t &callback_;
    std::lock_guard<std::recursive_mutex> _lock;
  };

  static std::ofstream fi;
  mutable std::shared_ptr<holder> _holder;
};