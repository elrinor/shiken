#!/bin/sh

PREFIX=`pwd`
SRC_DIR=$PREFIX/src

mkdir -p $SRC_DIR
cd $SRC_DIR

hg clone https://code.google.com/p/acvlib/ acvlib

hg clone https://code.google.com/p/arxlib/ arxlib

wget -c http://bitbucket.org/eigen/eigen/get/2.0.15.tar.bz2 -O eigen.tar.bz2
tar xjf eigen.tar.bz2

wget -c http://hci.iwr.uni-heidelberg.de/vigra/vigra-1.7.1-src.tar.gz
tar xzf vigra-1.7.1-src.tar.gz

wget -c http://surfnet.dl.sourceforge.net/project/boost/boost/1.46.0/boost_1_46_0.tar.bz2
tar xjf boost_1_46_0.tar.bz2

# Notes: bjam must be installed (from distributive repository)
cd boost_1_46_0
bjam --build-type=complete --layout=tagged
cd ..

wget -c http://get.qt.nokia.com/qt/source/qt-everywhere-opensource-src-4.7.1.tar.gz
tar xzf qt-everywhere-opensource-src-4.7.1.tar.gz

# Notes: libxext-dev, libfontconfig-dev must be installed.
cd qt-everywhere-opensource-src-4.7.1

cat << EOF > src/plugins/imageformats/imageformats.pro
TEMPLATE = subdirs

!contains(QT_CONFIG, no-jpeg):contains(QT_CONFIG, jpeg):SUBDIRS += jpeg
!contains(QT_CONFIG, no-gif):contains(QT_CONFIG, gif):SUBDIRS += gif
!contains(QT_CONFIG, no-mng):contains(QT_CONFIG, mng):SUBDIRS += mng
contains(QT_CONFIG, svg):SUBDIRS += svg
!contains(QT_CONFIG, no-tiff):contains(QT_CONFIG, tiff):SUBDIRS += tiff
!contains(QT_CONFIG, no-ico):SUBDIRS += ico
EOF

cat << EOF > src/plugins/sqldrivers/sqldrivers.pro
TEMPLATE = subdirs

contains(sql-plugins, psql)    : SUBDIRS += psql
contains(sql-plugins, mysql)    : SUBDIRS += mysql
contains(sql-plugins, odbc)    : SUBDIRS += odbc
contains(sql-plugins, tds)    : SUBDIRS += tds
contains(sql-plugins, oci)    : SUBDIRS += oci
contains(sql-plugins, db2)    : SUBDIRS += db2
contains(sql-plugins, sqlite)    : SUBDIRS += sqlite
SUBDIRS += sqlite
contains(sql-plugins, sqlite2)    : SUBDIRS += sqlite2
contains(sql-plugins, ibase)    : SUBDIRS += ibase

symbian:contains(CONFIG, system-sqlite): SUBDIRS += sqlite_symbian
EOF

echo o | ./configure -static -no-webkit -nomake examples -nomake demos -confirm-license -prefix $PREFIX
nice -j2 make install
cd ..

# Notes: g++-4.5 must be installed (from debian-experimental, for example)
svn co --username=xxx --password=xxx https://xxx.xxx/barcode barcode
cd barcode
export QTDIR=$PREFIX
export ARXDIR=$SRC_DIR/arxlib/include
export CPLUS_INCLUDE_PATH=$SRC_DIR/acvlib/include:$ARXDIR:$SRC_DIR/boost_1_46_0:$SRC_DIR/eigen:$SRC_DIR/vigra-1.7.1/include
export LIBRARY_PATH=$SRC_DIR/boost_1_46_0/stage/lib
$PREFIX/bin/qmake "CONFIG+=static" QMAKE_CXX=g++-4.5 htmlconv.pro && make
$PREFIX/bin/qmake "CONFIG+=static" QMAKE_CXX=g++-4.5 scanrec.pro && make
$PREFIX/bin/qmake "CONFIG+=static" QMAKE_CXX=g++-4.5 shiken.pro && make
$PREFIX/bin/qmake "CONFIG+=static" QMAKE_CXX=g++-4.5 protrec.pro && make
cd ..
