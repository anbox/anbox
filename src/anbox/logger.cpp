/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <thread>

#ifndef BOOST_LOG_DYN_LINK
#define BOOST_LOG_DYN_LINK
#endif
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators.hpp>
#include <boost/log/utility/setup.hpp>
#define BOOST_LOG_USE_NATIVE_SYSLOG
#include <boost/log/sinks/syslog_backend.hpp>

#include "anbox/logger.h"

namespace {
namespace attrs {
BOOST_LOG_ATTRIBUTE_KEYWORD(Severity, "Severity", anbox::Logger::Severity)
BOOST_LOG_ATTRIBUTE_KEYWORD(Location, "Location", anbox::Logger::Location)
BOOST_LOG_ATTRIBUTE_KEYWORD(Timestamp, "Timestamp", boost::posix_time::ptime)
}

struct BoostLogLogger : public anbox::Logger {
  BoostLogLogger() : initialized_(false) {}

  void Init(const anbox::Logger::Severity& severity = anbox::Logger::Severity::kWarning) override {
    if (initialized_)
      return;

    boost::log::formatter formatter =
        boost::log::expressions::stream
        << "[" << attrs::Severity << " "
        << boost::log::expressions::format_date_time<boost::posix_time::ptime>(
               "Timestamp", "%Y-%m-%d %H:%M:%S")
        << "] "
        << boost::log::expressions::if_(boost::log::expressions::has_attr(
               attrs::Location))[boost::log::expressions::stream
                                 << "[" << attrs::Location << "] "]
        << boost::log::expressions::smessage;

    boost::log::core::get()->remove_all_sinks();

    // If we have a controlling tty then we use the console for log outpu
    // and otherwise we move everything into the system syslog.
    if (isatty(0)) {
      auto logger = boost::log::add_console_log(std::cout);
      logger->set_formatter(formatter);
    } else {
      boost::shared_ptr<boost::log::sinks::syslog_backend> backend(
            new boost::log::sinks::syslog_backend(
              boost::log::keywords::facility = boost::log::sinks::syslog::user,
              boost::log::keywords::use_impl = boost::log::sinks::syslog::native));
      backend->set_severity_mapper(boost::log::sinks::syslog::direct_severity_mapping<int>("Severity"));
      boost::log::core::get()->add_sink(boost::make_shared<boost::log::sinks::synchronous_sink<
                                          boost::log::sinks::syslog_backend>>(backend));
    }

    severity_ = severity;
    initialized_ = true;
  }

  void SetSeverity(const Severity& severity) override {
    severity_ = severity;
  }

  Severity GetSeverity() override {
    return severity_;
  }

  void Log(Severity severity, const std::string& message, const boost::optional<Location>& loc) override {
    if (!initialized_) Init();

    // FIXME somehow set_filter doesn't work with the trivial logger. If
    // we set a filter based on the severity attribute open_record will
    // not return a new record. Because of that we do a poor man filtering
    // here until we have a proper way to do this via boost.
    if (severity < severity_)
      return;

    if (auto rec = boost::log::trivial::logger::get().open_record()) {
      boost::log::record_ostream out{rec};
      out << boost::log::add_value(attrs::Severity, severity)
          << boost::log::add_value(attrs::Timestamp, boost::posix_time::microsec_clock::universal_time())
          << message;

      if (loc) {
        // We have to pass in a temporary as boost::log (<= 1.55) expects a
        // mutable reference to be passed to boost::log::add_value(...).
        auto tmp = *loc;
        out << boost::log::add_value(attrs::Location, tmp);
      }

      boost::log::trivial::logger::get().push_record(std::move(rec));
    }
  }

 private:
  Severity severity_;
  bool initialized_;
};

std::shared_ptr<anbox::Logger>& MutableInstance() {
  static std::shared_ptr<anbox::Logger> instance{new BoostLogLogger()};
  return instance;
}

void SetInstance(const std::shared_ptr<anbox::Logger>& logger) {
  MutableInstance() = logger;
}
}
namespace anbox {

bool Logger::SetSeverityFromString(const std::string& severity) {
  if (severity == "trace")
    SetSeverity(Severity::kTrace);
  else if (severity == "debug")
    SetSeverity(Severity::kDebug);
  else if (severity == "info")
    SetSeverity(Severity::kInfo);
  else if (severity == "warning")
    SetSeverity(Severity::kWarning);
  else if (severity == "error")
    SetSeverity(Severity::kError);
  else if (severity == "fatal")
    SetSeverity(Severity::kFatal);
  else
    return false;
  return true;
}

void Logger::Trace(const std::string& message,
                   const boost::optional<Location>& location) {
  Log(Severity::kTrace, message, location);
}

void Logger::Debug(const std::string& message,
                   const boost::optional<Location>& location) {
  Log(Severity::kDebug, message, location);
}

void Logger::Info(const std::string& message,
                  const boost::optional<Location>& location) {
  Log(Severity::kInfo, message, location);
}

void Logger::Warning(const std::string& message,
                     const boost::optional<Location>& location) {
  Log(Severity::kWarning, message, location);
}

void Logger::Error(const std::string& message,
                   const boost::optional<Location>& location) {
  Log(Severity::kError, message, location);
}

void Logger::Fatal(const std::string& message,
                   const boost::optional<Location>& location) {
  Log(Severity::kFatal, message, location);
}

std::ostream& operator<<(std::ostream& strm, anbox::Logger::Severity severity) {
  switch (severity) {
    case anbox::Logger::Severity::kTrace:
      return strm << "TT";
    case anbox::Logger::Severity::kDebug:
      return strm << "DD";
    case anbox::Logger::Severity::kInfo:
      return strm << "II";
    case anbox::Logger::Severity::kWarning:
      return strm << "WW";
    case anbox::Logger::Severity::kError:
      return strm << "EE";
    case anbox::Logger::Severity::kFatal:
      return strm << "FF";
    default:
      return strm << static_cast<uint>(severity);
  }
}

std::ostream& operator<<(std::ostream& out, const Logger::Location& location) {
  return out << utils::string_format(
             "%s:%d@%s",
             boost::filesystem::path(location.file).filename().string(),
             location.line, location.function);
}

Logger& Log() { return *MutableInstance(); }

void SetLogger(const std::shared_ptr<Logger>& logger) { SetInstance(logger); }

}  // namespace anbox
