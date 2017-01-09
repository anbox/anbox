/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* this program is used to read a set of system properties and their values
 * from the emulator program and set them in the currently-running emulated
 * system. It does so by connecting to the 'boot-properties' qemud service.
 *
 * This program should be run as root and called from
 * /system/etc/init.goldfish.rc exclusively.
 */

#define LOG_TAG  "qemu-props"

#define DEBUG  1

#if DEBUG
#  include <cutils/log.h>
#  define  DD(...)    ALOGI(__VA_ARGS__)
#else
#  define  DD(...)    ((void)0)
#endif

#include <cutils/properties.h>
#include <unistd.h>
#include <hardware/qemud.h>

/* Name of the qemud service we want to connect to.
 */
#define  QEMUD_SERVICE  "boot-properties"

#define  MAX_TRIES      5

int  main(void)
{
    int  qemud_fd, count = 0;

    /* try to connect to the qemud service */
    {
        int  tries = MAX_TRIES;

        while (1) {
            qemud_fd = qemud_channel_open( "boot-properties" );
            if (qemud_fd >= 0)
                break;

            if (--tries <= 0) {
                DD("Could not connect after too many tries. Aborting");
                return 1;
            }

            DD("waiting 1s to wait for qemud.");
            sleep(1);
        }
    }

    DD("connected to '%s' qemud service.", QEMUD_SERVICE);

    /* send the 'list' command to the service */
    if (qemud_channel_send(qemud_fd, "list", -1) < 0) {
        DD("could not send command to '%s' service", QEMUD_SERVICE);
        return 1;
    }

    /* read each system property as a single line from the service,
     * until exhaustion.
     */
    for (;;)
    {
#define  BUFF_SIZE   (PROPERTY_KEY_MAX + PROPERTY_VALUE_MAX + 2)
        DD("receiving..");
        char* q;
        char  temp[BUFF_SIZE];
        int   len = qemud_channel_recv(qemud_fd, temp, sizeof temp - 1);

        /* lone NUL-byte signals end of properties */
        if (len < 0 || len > BUFF_SIZE-1 || temp[0] == '\0')
            break;

        temp[len] = '\0';  /* zero-terminate string */

        DD("received: %.*s", len, temp);

        /* separate propery name from value */
        q = strchr(temp, '=');
        if (q == NULL) {
            DD("invalid format, ignored.");
            continue;
        }
        *q++ = '\0';

        if (property_set(temp, q) < 0) {
            DD("could not set property '%s' to '%s'", temp, q);
        } else {
            count += 1;
        }
    }


    /* finally, close the channel and exit */
    close(qemud_fd);
    DD("exiting (%d properties set).", count);
    return 0;
}
