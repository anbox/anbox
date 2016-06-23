package org.anbox;

import android.app.Application;

public class MainApplication extends Application {
    private NavBar mNavBar;

    public void startServices() {
        mNavBar = new NavBar(this);
    }
}
