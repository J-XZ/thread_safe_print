#pragma once

#include <atomic>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace thread_safe_print {

class ThreadLocalLogBuilder {
 public:
  explicit ThreadLocalLogBuilder(const std::string &init_data) { ssx_.str(init_data); }

  void AddData(const std::string &data) {
    auto g = GetLockGuard_();
    ssx_ << data;
    std::string after_add = ssx_.str();
  }

  void AddData(const std::stringstream &ss) {
    auto g = GetLockGuard_();
    ssx_ << ss.str();
    std::string after_add = ssx_.str();
  }

  std::string GetData() {
    auto g = GetLockGuard_();
    return ssx_.str();
  }

  bool BeginWith(char a) {
    auto g        = GetLockGuard_();
    std::string c = ssx_.str();
    if (c.empty()) {
      return false;
    }
    return c[0] == a;
  }

  bool IfBeginWithThenAppend(char a, const std::string &data) {
    auto g        = GetLockGuard_();
    std::string c = ssx_.str();
    if (c.empty()) {
      return false;
    }
    if (c[0] == a) {
      ssx_ << data;
      return true;
    }
    return false;
  }

  bool IfBeginWithThenAppend(char a, const std::stringstream &ss) {
    auto g        = GetLockGuard_();
    std::string c = ssx_.str();
    if (c.empty()) {
      return false;
    }
    if (c[0] == a) {
      ssx_ << ss.str();
      std::string after_add = ssx_.str();
      return true;
    }
    return false;
  }

  void Clear() {
    auto g = GetLockGuard_();
    ssx_.str("");
  }

 private:
  void Lock_() {
    while (ssx_spin_mutex_.test_and_set(std::memory_order_acquire)) {
      ;
    }
  }

  void Unlock_() { ssx_spin_mutex_.clear(std::memory_order_release); }

  class LockGuard {
   public:
    explicit LockGuard(ThreadLocalLogBuilder *builder) : builder_(builder) { builder_->Lock_(); }

    ~LockGuard() { builder_->Unlock_(); }

   private:
    ThreadLocalLogBuilder *builder_;
  };

  std::unique_ptr<LockGuard> GetLockGuard_() { return std::make_unique<LockGuard>(this); }

  std::stringstream ssx_;
  std::atomic_flag ssx_spin_mutex_ = ATOMIC_FLAG_INIT;
};

extern thread_local std::stringstream global_ss, global_ss_tmp;
extern thread_local bool to_err, to_alert;

static void PrintFinal() {
  global_ss << std::endl;  // NOLINT
  if (to_err) {
    if (!to_alert) {
      std::stringstream ss;
      ss << "\033[31m" << global_ss.str() << "\033[0m";
      std::cout << ss.str() << std::flush;
    } else {
      std::stringstream ss;
      ss << "\033[33m" << global_ss.str() << "\033[0m";
      std::cout << ss.str() << std::flush;
    }
  } else {
    std::cout << global_ss.str() << std::flush;
  }
}

static void ToStringArgs() {};

static auto ToString(const char *value) -> std::string {
  if (value != nullptr) {
    return {value};
  }
  return "NULL";
}

static auto ToString(const std::vector<std::string> &v) -> std::string {
  std::stringstream ss;
  ss << "[";
  for (const auto &s : v) {
    ss << s;
    if (&s != &v[v.size() - 1]) {
      ss << ", ";
    }
  }
  ss << "]";
  return ss.str();
}

template <class T>
static auto ToString(const std::vector<T> &v) -> std::string {
  std::stringstream ss;
  ss << "[";
  for (const auto &s : v) {
    ss << s;
    if (&s != &v[v.size() - 1]) {
      ss << ", ";
    }
  }
  ss << "]";
  return ss.str();
}

template <typename T>
auto ToString(const T &value) -> std::string {
  std::stringstream oss{};
  oss << value;
  auto ret = oss.str();
  if (ret[0] == '\n') {
    ret.erase(0, 1);
  }
  return ret;
}

template <typename T, typename... Args>
void ToStringArgs(const T &value, const Args &...args) {
  auto s           = ToString(value);
  std::string link = " ";
  if (s[0] == '\n') {
    link = "\n";
    s.erase(0, 1);
  }
  if (global_ss_tmp.str()[global_ss_tmp.str().size() - 1] == '\n') {
    link = "";
  }
  if (global_ss_tmp.str()[global_ss_tmp.str().size() - 1] == ' ') {
    link = "";
  }

  global_ss_tmp << link << s;
  ToStringArgs(args...);
}

template <typename T, typename... Args>
auto MyToString(const T &value, const Args &...args) -> std::string {
  global_ss_tmp.str("");
  global_ss_tmp.clear();

  auto s           = ToString(value);
  std::string link = " ";
  if (global_ss_tmp.str().empty()) {
    link = "";
  }
  if (s[0] == '\n') {
    link = "\n";
    s.erase(0, 1);
  }
  if (!global_ss_tmp.str().empty() && global_ss_tmp.str()[global_ss_tmp.str().size() - 1] == '\n') {
    link = "";
  }

  global_ss_tmp << link << s;

  ToStringArgs(args...);
  return global_ss_tmp.str();
}

constexpr const char *kMyLogHead = "";

template <typename T, typename... Args>
void PrepareBuffer(const T &value, const Args &...args) {
  global_ss.clear();
  global_ss.str("");
  global_ss << kMyLogHead;
  global_ss << MyToString(value, args...);  // NOLINT
  PrintFinal();
}

static void ThreadSafePrintEmptyLine() { std::cout << "" << std::flush; }

template <typename T, typename... Args>
void ThreadSafePrintLine(const T &value, const Args &...args) {
  to_err   = false;
  to_alert = false;
  PrepareBuffer(value, args...);
}

template <typename T, typename... Args>
void ThreadSafePrintLineToStdErr(const T &value, const Args &...args) {
  to_err   = true;
  to_alert = false;

  PrepareBuffer(value, args...);
}

template <typename T, typename... Args>
void ThreadSafePrintLineToAllert(const T &value, const Args &...args) {
  to_err   = true;
  to_alert = true;

  PrepareBuffer(value, args...);
}

static std::string Uint64ToBinaryString(uint64_t value) {
  const int k_max_bit = 64;
  std::bitset<k_max_bit> bs{value};
  return bs.to_string();
}

}  // namespace thread_safe_print
