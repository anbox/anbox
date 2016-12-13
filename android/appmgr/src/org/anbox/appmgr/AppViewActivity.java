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

import android.os.Bundle;
import android.util.Log;
import android.support.v4.app.FragmentActivity;

public final class AppViewActivity extends FragmentActivity {
    private static final String TAG = "AnboxAppView";

    @Override
    public void onCreate(Bundle info) {
        super.onCreate(info);

        setContentView(R.layout.app_view);

        Log.i(TAG, "Created application view activity");
    }

    @Override
    public void onDestroy() {
        Log.i(TAG, "Destroying application view activity");
        super.onDestroy();
    }
}
