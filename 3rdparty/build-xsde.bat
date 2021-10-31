cd xsde/libxsde

cp ../config/config.nmake ../config/config-original.nmake 

echo XSDE_LONGLONG = y >> ../config/config.nmake
echo XSDE_IOSTREAM = n >> ../config/config.nmake
echo CFLAGS = $(CFLAGS) /Dstrtoull=_strtoui64 >> ../config/config.nmake
echo CXXFLAGS = $(CXXFLAGS) /Dstrtoull=_strtoui64 >> ../config/config.nmake

cp ../config/config.nmake ../config/config-temp.nmake 

echo CFLAGS = $(CFLAGS) /MT >>../config/config.nmake
echo CXXFLAGS = $(CXXFLAGS) /MT >>../config/config.nmake
nmake -f nmakefile
mv xsde/xsde.lib xsde/xsde-mt.lib
cp ../config/config-temp.nmake ../config/config.nmake

echo CFLAGS = $(CFLAGS) /MTd /Zi /Fdxsde-mtd.pdb >>../config/config.nmake
echo CXXFLAGS = $(CXXFLAGS) /MTd /Zi /Fdxsde-mtd.pdb >>../config/config.nmake
echo LDFLAGS  = $(LDFLAGS) /DEBUG >>../config/config.nmake
nmake -f nmakefile
mv xsde/xsde.lib xsde/xsde-mtd.lib
cp ../config/config-temp.nmake ../config/config.nmake

echo CFLAGS = $(CFLAGS) /MD >>../config/config.nmake
echo CXXFLAGS = $(CXXFLAGS) /MD >>../config/config.nmake
nmake -f nmakefile
mv xsde/xsde.lib xsde/xsde-md.lib
cp ../config/config-temp.nmake ../config/config.nmake

echo CFLAGS = $(CFLAGS) /MDd /Zi /Fdxsde-mdd.pdb >>../config/config.nmake
echo CXXFLAGS = $(CXXFLAGS) /MDd /Zi /Fdxsde-mdd.pdb >>../config/config.nmake
echo LDFLAGS  = $(LDFLAGS) /DEBUG >>../config/config.nmake
nmake -f nmakefile
mv xsde/xsde.lib xsde/xsde-mdd.lib
cp ../config/config-temp.nmake ../config/config.nmake

cp ../config/config-original.nmake ../config/config.nmake 

cd ../..
