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
import android.graphics.drawable.Drawable;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;

import java.util.List;
import java.io.ByteArrayOutputStream;

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

    public void notifyPackageRemoved(String packageName) {
        connectService();

        if (mService == null)
            return;

        Log.i(TAG, "Sending package removed notification to host service");

        Parcel data = Parcel.obtain();
        data.writeInterfaceToken(DESCRIPTOR);
        // No added or updated applications to report
        data.writeInt(0);
        // .. but a single removed application
        data.writeInt(1);
        data.writeString(packageName);

        Parcel reply = Parcel.obtain();
        try {
            mService.transact(TRANSACTION_updateApplicationList, data, reply, 0);
        }
        catch (RemoteException ex) {
            Log.w(TAG, "Failed to send updatePackageList request to remote binder service: " + ex.getMessage());
        }
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

            Intent launchIntent = mPm.getLaunchIntentForPackage(appInfo.packageName);
            if (launchIntent == null)
                continue;

            Drawable icon = null;
            try {
                icon = mPm.getApplicationIcon(appInfo.packageName);
            }
            catch (PackageManager.NameNotFoundException ex) {
                continue;
            }

            if (icon == null)
                continue;

            String name = appInfo.name;
            CharSequence label = appInfo.loadLabel(mPm);
            if (label != null)
                name = label.toString();

            data.writeString(name);
            data.writeString(appInfo.packageName);

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

            Bitmap iconBitmap = drawableToBitmap(icon);
            ByteArrayOutputStream outStream = new ByteArrayOutputStream();
            iconBitmap.compress(Bitmap.CompressFormat.PNG, 90, outStream);
            data.writeByteArray(outStream.toByteArray());
        }

        // We don't have any removed applications to include in the update
        data.writeInt(0);

        Parcel reply = Parcel.obtain();
        try {
            mService.transact(TRANSACTION_updateApplicationList, data, reply, 0);
        }
        catch (RemoteException ex) {
            Log.w(TAG, "Failed to send updatePackageList request to remote binder service: " + ex.getMessage());
        }
    }

    private Bitmap drawableToBitmap(Drawable drawable) {
        if (drawable instanceof BitmapDrawable)
            return ((BitmapDrawable)drawable).getBitmap();

        Bitmap bitmap = Bitmap.createBitmap(drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight(), Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        drawable.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
        drawable.draw(canvas);

        return bitmap;
    }
}
