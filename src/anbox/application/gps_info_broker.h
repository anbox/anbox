#ifndef ANBOX_APPLICATION_GPS_INFO_BROKER_H_
#define ANBOX_APPLICATION_GPS_INFO_BROKER_H_

#include <boost/signals2.hpp>
#include <string>

#include "anbox/do_not_copy_or_move.h"

namespace anbox::application {
struct GpsInfoBroker : public DoNotCopyOrMove {
  boost::signals2::signal<void(const std::string&)> newNmeaSentence;
};
}
#endif
