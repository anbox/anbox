/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
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

package org.anbox.app_manager;

import java.lang.reflect.Method;

import android.app.Application;
import android.os.IBinder;

public class MainApplication extends Application {
    public void startServices() {
        Method addService;
        try {
            addService = Class.forName("android.os.ServiceManager").getMethod("addService", String.class, IBinder.class);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
}