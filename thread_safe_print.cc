#include "thread_safe_printx.h"
#include <sstream>

namespace thread_safe_print {
thread_local std::stringstream global_ss{}, global_ss_tmp{};
thread_local bool to_err{}, to_alert{};
} // namespace thread_safe_print
