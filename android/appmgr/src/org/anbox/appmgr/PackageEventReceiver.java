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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;

public class PackageEventReceiver extends BroadcastReceiver {
    private static final String TAG = "AnboxAppMgr";

    private PlatformService mPlatformService;

    private String getPackageName(Intent intent) {
        Uri uri = intent.getData();
        String package_name = (uri != null ? uri.getSchemeSpecificPart() : null);
        return package_name;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (mPlatformService == null)
            mPlatformService = new PlatformService(context);

        if (intent.getAction() == Intent.ACTION_PACKAGE_ADDED ||
            intent.getAction() == Intent.ACTION_PACKAGE_CHANGED) {
            // Send updated list of applications to the host so that it
            // can update the list of applications available for the user.
            mPlatformService.sendApplicationListUpdate();
        } else if (intent.getAction() == Intent.ACTION_PACKAGE_REMOVED) {
            // Only send notification when package got removed and not replaced
            if (!intent.getBooleanExtra(Intent.EXTRA_REPLACING, false)) {
                mPlatformService.notifyPackageRemoved(getPackageName(intent));
            }
        }
    }
}
