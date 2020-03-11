Name:       anbox-sailfishos
Summary:    Run Android inside a container
Version:    1.1
Release:    1
Group:      System/Applications
License:    LGPL 2.1
URL:        https://github.com/sreehax/anbox
Source:     %{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  SDL2-devel
BuildRequires:  SDL2_image-devel
BuildRequires:  dbus-cpp-devel
BuildRequires:  lxc-devel
BuildRequires:  wayland-devel
BuildRequires:  wayland-egl-devel
BuildRequires:  protobuf-compiler
BuildRequires:  boost-devel
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(sdl2)
BuildRequires:  pkgconfig(libcap)
BuildRequires:  pkgconfig(audioresource)

Requires:  binutils
Requires:  boost-filesystem
Requires:  boost-iostreams
Requires:  boost-system
Requires:  boost-thread
Requires:  boost-program-options
Requires:  anbox-sailfishos-image
Requires:  boost-log
Requires:  lxc
Requires:  protobuf

%define debug_package %{nil}

%description
  Runtime for Android applications which runs a full Android system
  in a container using Linux namespaces (user, ipc, net, mount) to
  separate the Android system fully from the host.

%prep
%setup -q -n %{name}-%{version}

%build
mkdir -p build
cd build
%cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBDIR=lib -DBUILD_SHARED_LIBS=True -DCMAKE_BUILD_TYPE=None -DWerror=OFF

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
pushd build
%make_install
popd

install -Dm 755 scripts/prepare-anbox.sh %{buildroot}/%{_bindir}/prepare-anbox
install -Dm 755 scripts/anbox-bridge.sh %{buildroot}/%{_bindir}/anbox-bridge.sh
install -Dm 755 scripts/anbox.sh %{buildroot}/%{_bindir}/anbox.sh
install -Dm 755 scripts/anbox-shell.sh %{buildroot}/%{_bindir}/anbox-shell.sh
install -Dm 644 %{_sourcedir}/anbox-container-manager.service %{buildroot}/%{_unitdir}/anbox-container-manager.service
install -Dm 644 %{_sourcedir}/anbox-session-manager.service %{buildroot}/%{_libdir}/systemd/user/anbox-session-manager.service
install -Dm 644 %{_sourcedir}/anbox.desktop %{buildroot}/%{_datadir}/applications/anbox.desktop
install -Dm 644 snap/gui/icon.png %{buildroot}/%{_datadir}/icons/hicolor/512x512/apps/anbox.png

%post
if [ "$1" -ge 1 ]; then
   systemctl-user daemon-reload || true
   systemctl daemon-reload || true
  # systemctl-user restart anbox-session-manager.service || true
fi
prepare-anbox

%postun
if [ "$1" -eq 0 ]; then
   systemctl-user stop anbox-session-manager.service || true
   systemctl stop anbox-container-manager.service || true
   systemctl-user daemon-reload || true
   systemctl daemon-reload || true
fi

%files
%defattr(-,root,root,-)
#%doc README COPYING
%{_bindir}/anbox
%{_bindir}/anbox.sh
%{_bindir}/prepare-anbox
%{_bindir}/anbox-bridge.sh
%{_bindir}/anbox-shell.sh

%{_unitdir}/anbox-container-manager.service
%{_libdir}/systemd/user/anbox-session-manager.service
%{_datadir}/applications/anbox.desktop
%{_datadir}/icons/hicolor/512x512/apps/anbox.png
%{_datadir}/anbox/ui/loading-screen.png
