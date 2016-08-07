%define name bchunk
%define version 1.2.0
%define release 0

Summary: A CD image format converter from .bin/.cue to .iso/.cdr/.wav.
Name: %{name}
Version: %{version}
Release: %{release}
Group: Applications/Archiving
Copyright: GPL
Url: http://he.fi/bchunk/
Source: %{name}-%{version}.tar.gz
Buildroot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description
The bchunk package contains a UNIX/C rewrite of the BinChunker program.
BinChunker converts a CD image in a .bin/.cue format (sometimes .raw/.cue)
into a set of .iso and .cdr/.wav tracks.  The .bin/.cue format is used by some
non-UNIX CD-writing software, but is not supported on most other
CD-writing programs.

%prep
%setup -q

%build
CFLAGS="$RPM_OPT_FLAGS"
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_bindir}
mkdir -p $RPM_BUILD_ROOT%{_mandir}/man1
install -s -m 755 bchunk $RPM_BUILD_ROOT%{_bindir}
install -m 644 bchunk.1 $RPM_BUILD_ROOT%{_mandir}/man1

%clean
rm -r $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc COPYING README bchunk-%{version}.lsm
%{_bindir}/bchunk
%{_mandir}/man1/bchunk.1.gz

%changelog
* Tue Jun 29 2004 Hessu <hessu@hes.iki.fi>
- updated to 1.2.0

* Tue Aug 7 2001 Hessu <hessu@hes.iki.fi>
- updated to 1.1.1

* Fri Jul 13 2001 Christian W. Zuckschwerdt <zany@triq.net>
- replaced the hardcoded paths with macros

* Sun Jul 1 2001 Hessu <hessu@hes.iki.fi>
- updated to 1.1.0

* Mon Jul 24 2000 Prospector <prospector@redhat.com>
- rebuilt

* Mon Jul 10 2000 Tim Powers <timp@redhat.com>
- rebuilt

* Mon Jul 03 2000 Prospector <bugzilla@redhat.com>
- automatic rebuild

* Wed May 3 2000 Tim Powers <timp@redhat.com>
- rebuilt for 7.0

* Tue Jan 4 2000 Tim Powers <timp@redhat.com>
- rebuilt for 6.2

* Fri Jul 9 1999 Tim Powers <timp@redhat.com>
- added %defattr line and removed old %attr line
- built for 6.1

* Mon Apr 26 1999 Michael Maher <mike@redhat.com>
- built package for 6.0

* Thu Nov  5 1998 Fryguy_ <fryguy@falsehope.com>
  [bchunk-1.0.0-1]
- Initial Release
