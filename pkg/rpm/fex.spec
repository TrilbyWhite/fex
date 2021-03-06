summary: Frequency Excursion Calculator
name: fex
version: 2
release: 1
license: GPL3
group: Applications/Science
source: https://github.com/BehaviorEnterprises/Fex.git
url: https://wiki.BehaviorEnterprises.com
vendor: Behavior Enterprises
packager: Jesse McClure jesse [at] mccluresk9 [dot] com
requires: cairo, desktop-file-utils, fftw, libsndfile, libXpm, python, pygtk2, sox
buildrequires: cairo-devel, fftw-devel, gcc, libsndfile-devel, libX11-devel, libXpm-devel, pkgconfig
prefix: /usr

%description
Frequency Excursion Calculator

%prep
%setup -c

%build
make

%install
make "DESTDIR=${RPM_BUILD_ROOT}" install

%clean
rm -rf "${RPM_BUILD_ROOT}"

%post
update-desktop-database -q

%postun
update-desktop-database -q

%files
/usr/bin/fex
/usr/bin/fex-gtk
/usr/share/applications/
/usr/share/applications/fex.desktop
/usr/share/fex/
/usr/share/fex/config
/usr/share/man/man1/fex-help.1.gz
/usr/share/man/man1/fex.1.gz
/usr/share/pixmaps/fex.png

