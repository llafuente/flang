#!/bin/bash

exit 0

echo "hello world!"

sh /vagrant/llvm.sh

cd /tmp
rm -rf string.c
git clone https://github.com/llafuente/string.c.git
cd string.c
sh autogen.sh
./configure --prefix /usr
make
make install

#maybe not needed...
echo "/usr/lib/libstringc" > /etc/ld.so.conf.d/libstringc.conf
ldconfig
