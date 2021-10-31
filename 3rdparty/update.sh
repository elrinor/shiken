#!/bin/sh

# acvlib
hg clone https://code.google.com/p/acvlib/ acvlib

# arxlib
hg clone https://code.google.com/p/arxlib/ arxlib

# eigen
wget -c http://bitbucket.org/eigen/eigen/get/2.0.15.tar.bz2 --no-check-certificate -O eigen.tar.bz2 
bunzip2 eigen.tar.bz2
tar xf eigen.tar
mv eigen-eigen-0938af7840b0 eigen

# vigra
wget -c http://hci.iwr.uni-heidelberg.de/vigra/vigra-1.7.1-src.tar.gz
tar xzf vigra-1.7.1-src.tar.gz
mv vigra-1.7.1 vigra

# zlib
wget -c http://zlib.net/zlib126.zip
unzip zlib126.zip
mv zlib-1.2.6 zlib

# xsde
wget -c http://www.codesynthesis.com/download/xsde/3.2/windows/i686/xsde-3.2.0-i686-windows.zip
unzip xsde-3.2.0-i686-windows.zip
mv xsde-3.2.0-i686-windows xsde


