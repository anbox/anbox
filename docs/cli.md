## **Session Manager**
> Run the the anbox session manager
```sh
anbox session-manager
```
| Argument  | Description |
| ------------- | ------------- |
| `--desktop_file_hint` | Desktop file hint for QtMir/Unity8 |
| `--single-window` | Start in single window mode. |
| `--window-size` | Size of the window in single window mode, e.g. --window-size=1024,768 |
| `--standalone` | Prevents the Container Manager from starting the default container (Experimental) |
| `--experimental` | Allows users to use experimental features |
| `--use-system-dbus` | Use system instead of session DBus |
| `--software-rendering` | Use software rendering instead of hardware accelerated GL rendering |
| `--no-touch-emulation` | Disable touch emulation applied on mouse inputs |
| `--server-side-decoration` | Prefer to use server-side decoration instead of client-side decoration |
____
&nbsp;


## **Container Manager**
> Start the container manager service
```sh
sudo anbox container-manager --privileged --daemon --data-path=${PWD}/anbox-data --android-image=${PWD}/android.img
```
| Argument  | Description |
| ------------- | ------------- |
| `--data-path=[PATH]`  | Path where the container and its data is stored |
| `--android-image=[PATH]`  | Path to the Android rootfs image file if not stored in the data path (the default value is`[data-path]/android.img`)  |
| `--privileged`  | Run Android container in privileged mode |
| `--daemon`  | Mark service as being started as systemd daemon |
| `--use-rootfs-overlay` | Use an overlay for the Android rootfs |
| `--force-squashfuse` | Force using squashfuse for mounting the Android rootfs |
| `--container-network-address` | Assign the specified network address to the Android container |
| `--container-network-gateway` | Assign the specified network gateway to the Android container |
| `--container-network-dns-servers` | Assign the specified DNS servers to the Android container |
____
&nbsp;

## **Launching Applications**
> Launch an Activity by sending an intent
```sh
anbox launch --package=org.anbox.appmgr --component=org.anbox.appmgr.AppViewActivity
```
| Argument  | Description |
| ------------- | ------------- |
| `--action=` | Action of the intent |
| `--type=` | MIME type for the intent |
| `--uri=` | URI used as data within the intent |
| `--package=` | Package the intent should go to |
| `--component=` | Component of a package the intent should go |
| `--stack=[default|fullscreen|freeform]` | Which window stack the activity should be started on |
| `--use-system-dbus` | Use system instead of session DBus |
____
&nbsp;


## **Check Features**
> Check that the host system supports all necessary features.
```sh
anbox check-features
```
____
&nbsp;
## **Version**
> Print the version of the daemon
```sh
anbox version
```
____
&nbsp;
## **Wait ready**
> Wait until the Android system has successfully booted
```
anbox wait-ready
```