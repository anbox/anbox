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

import android.os.ServiceManager;
import android.os.IBinder;
import android.os.Parcel;
import android.os.RemoteException;
import android.util.Log;
import android.content.Intent;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.pm.ApplicationInfo;
import android.net.Uri;

import java.util.List;

public final class PlatformService {
    private static final String TAG = "AnboxAppMgr";
    private static final String SERVICE_NAME = "org.anbox.PlatformService";
    private static final String DESCRIPTOR = "org.anbox.IPlatformService";

    private static final int TRANSACTION_updateApplicationList = (IBinder.FIRST_CALL_TRANSACTION + 2);

    private IBinder mService = null;
    private PackageManager mPm = null;

    private void connectService() {
        if (mService != null)
            return;
        mService = ServiceManager.getService(SERVICE_NAME);
    }

    public PlatformService(Context context) {
        if (context != null) {
            mPm = context.getPackageManager();
        } else {
            Log.w(TAG, "No context available");
        }

        Log.i(TAG, "Connected to platform service");
    }

    public void sendApplicationListUpdate() {
        connectService();

        if (mPm == null || mService == null)
            return;

        Parcel data = Parcel.obtain();
        data.writeInterfaceToken(DESCRIPTOR);

        List<ApplicationInfo> apps = mPm.getInstalledApplications(0);
        data.writeInt(apps.size());
        for (int n = 0; n < apps.size(); n++) {
            ApplicationInfo appInfo = apps.get(n);
            data.writeString(appInfo.name);
            data.writeString(appInfo.packageName);

            Intent launchIntent = mPm.getLaunchIntentForPackage(appInfo.packageName);
            if (launchIntent != null) {
                data.writeInt(1);
                data.writeString(launchIntent.getAction());
                if (launchIntent.getData() != null)
                    data.writeString(launchIntent.getData().toString());
                else
                    data.writeString("");
                data.writeString(launchIntent.getType());
                data.writeString(launchIntent.getComponent().getPackageName());
                data.writeString(launchIntent.getComponent().getClassName());
                data.writeInt(launchIntent.getCategories().size());
                for (String category : launchIntent.getCategories())
                    data.writeString(category);
            } else {
                data.writeInt(0);
            }

            // FIXME add icon, flags, ...
        }

        Parcel reply = Parcel.obtain();
        try {
            mService.transact(TRANSACTION_updateApplicationList, data, reply, 0);
        }
        catch (RemoteException ex) {
            Log.w(TAG, "Failed to send updatePackageList request to remote binder service: " + ex.getMessage());
        }
    }

    public void notifyPackageAdded(Intent intent) {
    }

    public void notifyPackageRemoved(Intent intent) {
    }
}
