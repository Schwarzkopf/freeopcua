/// @license GNU LGPL
///
/// Distributed under the GNU LGPL License
/// (See accompanying file LICENSE or copy at
/// http://www.gnu.org/licenses/lgpl.html)
///

#pragma once

#include <opc/common/class_pointers.h>
#ifdef HAVE_SYSTEM_SPDLOG
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>
#else
#include <opc/spdlog/spdlog.h>
#include <opc/spdlog/fmt/ostr.h>
#endif

// Since fmt 9, types are no longer formatted through operator<< automatically.
// Provide a formatter for every class type of the OpcUa namespace that can be
// streamed to std::ostream (found via ADL), so existing log calls keep working.
#if defined(FMT_VERSION) && FMT_VERSION >= 90000
#include <ostream>
#include <type_traits>
#include <utility>

namespace OpcUa
{
// Never defined: only used to detect (via ADL) types associated with OpcUa.
// ADL only searches the innermost namespace, so nested namespaces need it too.
template <typename T> std::true_type FreeOpcUaFmtAdlMarker(const T &);
namespace Binary { using OpcUa::FreeOpcUaFmtAdlMarker; }
namespace Internal { using OpcUa::FreeOpcUaFmtAdlMarker; }
namespace Model { using OpcUa::FreeOpcUaFmtAdlMarker; }
namespace Server { using OpcUa::FreeOpcUaFmtAdlMarker; }
}

namespace Common
{
namespace Logger
{
namespace Detail
{
template <typename...> struct MakeVoid { typedef void type; };
template <typename... Ts> using VoidT = typename MakeVoid<Ts...>::type;

template <typename T, typename = void> struct HasBegin : std::false_type {};
template <typename T>
struct HasBegin<T, VoidT<decltype(std::declval<const T &>().begin())>> : std::true_type {};

template <typename T, typename = void> struct IsOpcUaStreamable : std::false_type {};
template <typename T>
struct IsOpcUaStreamable<T, VoidT<
  decltype(FreeOpcUaFmtAdlMarker(std::declval<const T &>())),
  decltype(std::declval<std::ostream &>() << std::declval<const T &>())>>
  : std::integral_constant<bool, (std::is_class<T>::value || std::is_enum<T>::value) && !HasBegin<T>::value> {};

// OpcUa enums without operator<< are printed as their underlying integer (old fmt behaviour).
template <typename T, typename = void> struct IsOpcUaPlainEnum : std::false_type {};
template <typename T>
struct IsOpcUaPlainEnum<T, VoidT<decltype(FreeOpcUaFmtAdlMarker(std::declval<const T &>()))>>
  : std::integral_constant<bool, std::is_enum<T>::value && !IsOpcUaStreamable<T>::value> {};
} // namespace Detail
} // namespace Logger
} // namespace Common

template <typename T>
struct fmt::formatter<T, char, typename std::enable_if<Common::Logger::Detail::IsOpcUaStreamable<T>::value>::type>
  : fmt::ostream_formatter {};

template <typename T>
struct fmt::formatter<T, char, typename std::enable_if<Common::Logger::Detail::IsOpcUaPlainEnum<T>::value>::type>
  : fmt::formatter<typename std::underlying_type<T>::type>
{
  typedef typename std::underlying_type<T>::type Underlying;

  template <typename FormatContext>
  auto format(const T & value, FormatContext & ctx) const -> decltype(ctx.out())
  {
    return fmt::formatter<Underlying>::format(static_cast<Underlying>(value), ctx);
  }
};
#endif

namespace Common
{

namespace Logger {
  DEFINE_CLASS_POINTERS(spdlog::logger)
};

}

#define LOG_TRACE(__logger__, ...) { if (__logger__ && __logger__->should_log(spdlog::level::trace)) { __logger__->trace(__VA_ARGS__); }}
#define LOG_DEBUG(__logger__, ...) { if (__logger__ && __logger__->should_log(spdlog::level::debug)) { __logger__->debug(__VA_ARGS__); }}
#define LOG_INFO(__logger__, ...) { if (__logger__ && __logger__->should_log(spdlog::level::info)) { __logger__->info(__VA_ARGS__); }}
#define LOG_WARN(__logger__, ...) { if (__logger__ && __logger__->should_log(spdlog::level::warn)) { __logger__->warn(__VA_ARGS__); }}
#define LOG_ERROR(__logger__, ...) { if (__logger__ && __logger__->should_log(spdlog::level::err)) { __logger__->error(__VA_ARGS__); }}
#define LOG_CRITICAL(__logger__, ...) { if (__logger__ && __logger__->should_log(spdlog::level::critical)) { __logger__->critical(__VA_ARGS__); }}
