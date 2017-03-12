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

package org.anbox.appmgr;

import android.app.Service;
import android.util.Log;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;

public final class LauncherService extends Service {
    public static final String TAG = "AnboxAppMgr";

    private PlatformService mPlatformService;
    private PackageEventReceiver mPkgEventReceiver;

    public LauncherService() {
        super();
        Log.i(TAG, "Service created");
    }

    @Override
    public void onCreate() {
        mPlatformService = new PlatformService(getBaseContext());

        // Send the current list of applications over to the host so
        // it can rebuild its list of available applications.
        mPlatformService.sendApplicationListUpdate();

        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_PACKAGE_ADDED);
        filter.addAction(Intent.ACTION_PACKAGE_CHANGED);
        filter.addAction(Intent.ACTION_PACKAGE_REMOVED);
        filter.addDataScheme("package");

        mPkgEventReceiver = new PackageEventReceiver();
        registerReceiver(mPkgEventReceiver, filter);

        Log.i(TAG, "Service started");
    }

    @Override
    public void onDestroy() {
        Log.i(TAG, "Service stopped");
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
