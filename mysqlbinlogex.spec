Summary: mysqlbinlogex
Name: mysqlbinlogex
Version: 0.1.2
Release: 1
Group: Application/Database
Source0: mysqlbinlogex-%{version}.tar.gz
Vendor: Mastoshi Eizono
License: GPLv2
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot
Provides: mysqlbinlogex = %{version}

%description
mysqlbinlogex for mysql5 binary log parser.

%prep
%setup -q -n mysqlbinlogex-%{version}
#./buildconf

%build
# つくるくん。
./configure 

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
* Sat May 24 2014 Masatoshi Eizono <support@tapweb.co.jp>
- V0.1.1

