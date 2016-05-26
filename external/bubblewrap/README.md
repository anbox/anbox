Bubblewrap
==========

Many container runtime tools like `systemd-nspawn`, `docker`,
etc. focus on providing infrastructure for system administrators and
orchestration tools (e.g. Kubernetes) to run containers.

These tools are not suitable to give to unprivileged users, because it
is trivial to turn such access into to a fully privileged root shell
on the host.

User namespaces
---------------

There is an effort in the Linux kernel called
[user namespaces](https://www.google.com/search?q=user+namespaces+site%3Ahttps%3A%2F%2Flwn.net)
which attempts to allow unprivileged users to use container features.
While significant progress has been made, there are
[still concerns](https://lwn.net/Articles/673597/) about it, and
it is not available to unprivileged users in several production distributions
such as CentOS/Red Hat Enterprise Linux 7, Debian Jessie, etc.

See for example
[CVE-2016-3135](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2016-3135)
which is a local root vulnerability introduced by userns.
[This March 2016 post](https://lkml.org/lkml/2016/3/9/555) has some
more discussion.

Bubblewrap could be viewed as setuid implementation of a *subset* of
user namespaces.  Emphasis on subset - specifically relevant to the
above CVE, bubblewrap does not allow control over iptables.

The original bubblewrap code existed before user namespaces - it inherits code from
[xdg-app helper](https://cgit.freedesktop.org/xdg-app/xdg-app/tree/common/xdg-app-helper.c)
which in turn distantly derives from
[linux-user-chroot](https://git.gnome.org/browse/linux-user-chroot).

Security
--------

The maintainers of this tool believe that it does not, even when used
in combination with typical software installed on that distribution,
allow privilege escalation.  It may increase the ability of a logged
in user to perform denial of service attacks, however.

In particular, bubblewrap uses `PR_SET_NO_NEW_PRIVS` to turn off
setuid binaries, which is the [traditional way](https://en.wikipedia.org/wiki/Chroot#Limitations) to get out of things
like chroots.

Users
-----

This program can be shared by all container tools which perform
non-root operation, such as:

 - [xdg-app](https://cgit.freedesktop.org/xdg-app/xdg-app)
 - [rpm-ostree unprivileged](https://github.com/projectatomic/rpm-ostree/pull/209)

We would also like to see this be available in Kubernetes/OpenShift
clusters.  Having the ability for unprivileged users to use container
features would make it significantly easier to do interactive
debugging scenarios and the like.

Usage
-----

bubblewrap works by creating a new, completely empty, mount
namespace where the root is on a tmpfs that is invisible from the
host, and will be automatically cleaned up when the last process
exists. You can then use commandline options to construct the root
filesystem and process environment and command to run in the
namespace.

A simple example is
```
bwrap --ro-bind / / bash
```
This will create a read-only bind mount of the host root at the
sandbox root, and then start a bash.

Another simple example would be a read-write chroot operation:
```
bwrap --bind /some/chroot/dir / bash
```

A more complex example is to run a with a custom (readonly) /usr,
but your own (tmpfs) data, running in a PID and network namespace:

```
bwrap --ro-bind /usr /usr \
   --tmpfs /tmp \
   --proc /proc \
   --dev /dev \
   --ro-bind /etc/resolv.conf /etc/resolv.conf \
   --symlink usr/lib /lib \
   --symlink usr/lib64 /lib64 \
   --symlink usr/bin /bin \
   --symlink usr/sbin /sbin \
   --chdir / \
   --unshare-pid \
   --unshare-net \
   --dir /run/user/$(id -u) \
   --setenv XDG_RUNTIME_DIR "/run/user/`id -u`" \
   /bin/sh
```

Sandboxing
----------

The goal of bubblewrap is to run an application in a sandbox, where it
has restricted access to parts of the operating system or user data
such as the home directory.

bubblewrap always creates a new mount namespace, and the user can specify
exactly what parts of the filesystem should be visible in the sandbox.
Any such directories you specify mounted `nodev` by default, and can be made readonly.

Additionally you can use these kernel features:

User namespaces ([CLONE_NEWUSER](http://linux.die.net/man/2/clone)): This hides all but the current uid and gid from the
sandbox. You can also change what the value of uid/gid should be in the sandbox.

IPC namespaces ([CLONE_NEWIPC](http://linux.die.net/man/2/clone)): The sandbox will get its own copy of all the
different forms of IPCs, like SysV shared memory and semaphores.

PID namespaces ([CLONE_NEWPID](http://linux.die.net/man/2/clone)): The sandbox will not see any processes outside the sandbox. Additionally, bubblewrap will run a trivial pid1 inside your container to handle the requirements of reaping children in the sandbox. .This avoids what is known now as the [Docker pid 1 problem](https://blog.phusion.nl/2015/01/20/docker-and-the-pid-1-zombie-reaping-problem/).


Network namespaces ([CLONE_NEWNET](http://linux.die.net/man/2/clone)): The sandbox will not see the network. Instead it will have its own network namespace with only a loopback device.

UTS namespace ([CLONE_NEWUTS](http://linux.die.net/man/2/clone)): The sandbox will have its own hostname.

Seccomp filters: You can pass in seccomp filters that limit which syscalls can be done in the sandbox. For more information, see [Seccomp](https://en.wikipedia.org/wiki/Seccomp).

Related project comparison: Firejail
------------------------------------

[Firejail](https://github.com/netblue30/firejail/tree/master/src/firejail) is
similar to xdg-app before bubblewrap was split out in that it combines
a setuid tool with a lot of desktop-specific sandboxing features.  For
example, Firejail knows about Pulseaudio, whereas bubblewrap does not.

The bubblewrap authors believe it's much easier to audit a small
setuid program, and keep features such as Pulseaudio filtering as an
unprivileged process, as now occurs in xdg-app.

Also, @cgwalters thinks trying to
[whitelist file paths](https://github.com/netblue30/firejail/blob/37a5a3545ef6d8d03dad8bbd888f53e13274c9e5/src/firejail/fs_whitelist.c#L176)
is a bad idea given the myriad ways users have to manipulate paths,
and the myriad ways in which system administrators may configure a
system.  The bubblewrap approach is to only retain a few specific
Linux capabilities such as `CAP_SYS_ADMIN`, but to always access the
filesystem as the invoking uid.  This entirely closes
[TOCTOCU attacks](https://cwe.mitre.org/data/definitions/367.html) and
such.

Related project comparison: Sandstorm.io
----------------------------------------

[Sandstorm.io](https://sandstorm.io/) also has a setuid helper
process.  @cgwalters believes their setuid code is fairly good, but it
could still make sense to unify on bubblewrap as a setuid core.  That
hasn't been ruled out, but neither is it being actively pursued today.

Related project comparison: runc/binctr
----------------------------------------

[runc](https://github.com/opencontainers/runc) is similar to
[systemd nspawn](https://www.freedesktop.org/software/systemd/man/systemd-nspawn.html)
in that it is tooling intended to be invoked by root.  There is an
effort to have runc optionally use
[user namespaces](https://github.com/opencontainers/runc/issues/38),
but no plans for any setuid support.

The bubblewrap authors believe that runc and systemd-nspawn are not
designed to be made setuid and are distant from supporting such a
mode.

[binctr](https://github.com/jfrazelle/binctr) is just a wrapper for
runc, so inherits all of its design tradeoffs.

Whats with the name ?!
----------------------

The name bubblewrap was chosen to convey that this
tool runs as the parent of the application (so wraps it in some sense) and creates
a protective layer (the sandbox) around it.

![](bubblewrap.jpg)

(Bubblewrap cat by [dancing_stupidity](https://www.flickr.com/photos/27549668@N03/))
