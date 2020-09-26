# DBUS interface

Anbox uses DBUS interface for launching applications (`anbox launch` command) and for setting sensor values. You can query available interfaces using this command:
```
dbus-send --session --dest=org.anbox --type=method_call --print-reply /org/anbox org.freedesktop.DBus.Introspectable.Introspect
```
In particular you will see `org.anbox.ApplicationManager` and `org.anbox.Sensors` interfaces.

## Launching applications
Launching applications is done using `org.anbox.ApplicationManager` DBUS interface. To launch application you can use the following command:
```
anbox launch --package=org.anbox.appmgr --component=org.anbox.appmgr.AppViewActivity
```
This command internally uses DBUS to contact session manager and calls `Launch` method to launch specified component. Unfortunately due to limitations of `dbus-send` (in particular limited argument type support) this cannot be done using the generic dbus command line interface.

## Sensor values
You can get or set sensor values using `org.anbox.Sensors` interface. Note that there is no corresponding anbox command to use this interface.

### Available sensors
You can check available sensors using `dbus-send`:
```
dbus-send --session --dest=org.anbox --type=method_call --print-reply /org/anbox org.freedesktop.DBus.Introspectable.Introspect
```
At the time of this writing the following sensors are supported:
 - Acceleration
 - Humidity
 - Light
 - MagneticField
 - Orientation
 - Pressure
 - Proximity
 - Temperature

### Get/set using dbus-send
Use the following command to set temperature to 25.2 Celsius:
```
dbus-send --session --dest=org.anbox --print-reply /org/anbox org.freedesktop.DBus.Properties.Set string:org.anbox.Sensors string:Temperature variant:double:25.2
```
To get current sensor setting call:
```
dbus-send --session --dest=org.anbox --print-reply /org/anbox org.freedesktop.DBus.Properties.Get string:org.anbox.Sensors string:Temperature
```

### Get/set using Python
The `dbus-send` has limitations in its type support therefore to manipulate some sensor values you will need to use other tools. One of the availble tools is pydbus for Python. Here goes sample Python 3 script that changes composite value for orientation sensor.
```
#!/usr/bin/python3
from pydbus import SessionBus
bus = SessionBus()
proxy = bus.get("org.anbox", "/org/anbox")

proxy['org.anbox.Sensors'].Orientation=(proxy['org.anbox.Sensors'].Orientation[0]+1,0,0)
print (proxy['org.anbox.Sensors'].Orientation)
```
