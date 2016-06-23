package org.anbox;

import android.app.Application;

public class MainApplication extends Application {
    private NavBar mNavBar;

    static {
        // Load our native support library on startup so we get
        // access to several things from the Anbox host service.
        System.loadLibrary("anbox_support");
    }

    public void startServices() {
        mNavBar = new NavBar(this);
    }
}
