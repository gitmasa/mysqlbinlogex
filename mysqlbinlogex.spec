Summary: mysqlbinlogex
Name: mysqlbinlogex
Version: 0.3.1
Release: 1
Group: Application/Database
Source0: mysqlbinlogex-%{version}.tar.gz
Vendor: Mastoshi Eizono
License: GPLv2
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot
Provides: mysqlbinlogex = %{version}
require: boost-regex
require-dev: cmake > 2.8, boost-devel

%description
mysqlbinlogex for mysql5-6 binary log parser.

%prep
%setup -q -n mysqlbinlogex-%{version}

%build
# つくるくん。
cmake .

make clean
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/local/bin

make DESTDIR=$RPM_BUILD_ROOT install


%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/local/bin/mysqlbinlogex
/usr/local/bin/mysqltxtlog

%changelog
* Fri Apr 20 2018 Masatoshi Eizono <support@tapweb.co.jp>
- V0.3.1
* Thu Jul 28 2016 Masaki Hayashi <support@tapweb.co.jp>
- V0.2.0
* Sat May 24 2014 Masatoshi Eizono <support@tapweb.co.jp>
- V0.1.1

