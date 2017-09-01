Lxc Configuration for Hardware Resource Management
==================================================

By default, the containerized Android can see all hardware resource such as CPU cores, RAM and storage. If you'd like to restrict the resource that the containerized Android may access, a Lxc configuration can be defined and feed to Anbox container manager via `anbox container-manager --lxc-conf /path/to/config`.

Configuration Format
--------------------

One line one Lxc config.
Lines started with `#` are comments which will be ignored.
The config will be like `key = value` which will be parsed and filled to Lxc via `set_config_item(key, value)`.
A typical configuration is as below. 

```
# ignored lines
lxc.cgroup.cpuset.cpus = 0-1
```

Available Configurations
----------------------

Basically, all Lxc configurations are acceptable, for more configurations please refer to [Kernel](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/cgroup-v2.txt?h=v4.13-rc7) and [Lxc](https://linuxcontainers.org/).

Note that, with `lxc.cgroup.cpuset.cpus = 0-1` the container can still *see* all CPU cores though it can only *use* the configured ones - container is not VM anyway.

**Caution:** Your configuration may overwite anbox's default and lead to faulty, so be careful.
