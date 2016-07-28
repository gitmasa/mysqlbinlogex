Summary: mysqlbinlogex
Name: mysqlbinlogex
Version: 0.2.1
Release: 1
Group: Application/Database
Source0: mysqlbinlogex-%{version}.tar.gz
Vendor: Mastoshi Eizono
License: GPLv2
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot
Provides: mysqlbinlogex = %{version}

%description
mysqlbinlogex for mysql5-6 binary log parser.

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
* Thu Jul 28 2016 Masaki Hayashi <support@tapweb.co.jp>
- V0.2.0
* Sat May 24 2014 Masatoshi Eizono <support@tapweb.co.jp>
- V0.1.1

