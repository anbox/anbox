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

package org.anbox;

import android.app.ActivityManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;

public class NavBar extends Service {
    private NavigationBarView mNavigationBarView = null;
    private Context mContext;
    private WindowManager mWindowManager;

    public NavBar(Context context) {
        mContext = context;
        mNavigationBarView =
                (NavigationBarView) View.inflate(mContext, R.layout.navigation_bar, null);

        mWindowManager = (WindowManager)mContext.getSystemService(Context.WINDOW_SERVICE);
        mWindowManager.addView(mNavigationBarView, getNavigationBarLayoutParams());
        show();
    }

    private WindowManager.LayoutParams getNavigationBarLayoutParams() {
        WindowManager.LayoutParams lp = new WindowManager.LayoutParams(
                WindowManager.LayoutParams.MATCH_PARENT, WindowManager.LayoutParams.MATCH_PARENT,
                WindowManager.LayoutParams.FIRST_SYSTEM_WINDOW+19, //TYPE_NAVIGATION_BAR, Private API, which for some reason I can't include...
                0
                        | WindowManager.LayoutParams.FLAG_TOUCHABLE_WHEN_WAKING
                        | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                        | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
                        | WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH
                        | WindowManager.LayoutParams.FLAG_SPLIT_TOUCH,
                PixelFormat.TRANSLUCENT);
        // this will allow the navbar to run in an overlay on devices that support this
        lp.flags |= WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED;

        lp.setTitle("NavigationBar");
        lp.windowAnimations = 0;
        return lp;
    }

    public void show() {
        WindowManager.LayoutParams lp =
                (android.view.WindowManager.LayoutParams) mNavigationBarView.getLayoutParams();
        lp.flags &= ~WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL;


        mWindowManager.updateViewLayout(mNavigationBarView, lp);
    }

    public void hide() {
        WindowManager.LayoutParams lp =
                (android.view.WindowManager.LayoutParams) mNavigationBarView.getLayoutParams();
        lp.flags |= WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL;
        mWindowManager.updateViewLayout(mNavigationBarView, lp);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
