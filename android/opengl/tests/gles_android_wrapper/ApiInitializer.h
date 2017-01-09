/*
* Copyright 2011 The Android Open Source Project
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

#ifndef _API_INITIALIZER_H_
#define _API_INITIALIZER_H_
#include <stdlib.h>
#include <dlfcn.h>

class ApiInitializer {
public:
    ApiInitializer(void *dso) :
        m_dso(dso) {
    }
    static void *s_getProc(const char *name, void *userData) {
        ApiInitializer *self = (ApiInitializer *)userData;
        return self->getProc(name);
    }
private:
    void *m_dso;
    void *getProc(const char *name) {
        void *symbol = NULL;
        if (m_dso) {
            symbol = dlsym(m_dso, name);
        }
        return symbol;
    }
};

#endif
