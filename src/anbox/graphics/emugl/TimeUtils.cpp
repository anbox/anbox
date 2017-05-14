/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "anbox/graphics/emugl/TimeUtils.h"

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

long long GetCurrentTimeMS() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  long long iDiff = (now.tv_sec * 1000LL) + now.tv_nsec / 1000000LL;
  return iDiff;
}

void TimeSleepMS(int p_mili) { usleep(p_mili * 1000); }
