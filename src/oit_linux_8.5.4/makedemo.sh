for THISFILE in `ls redist`
do
    ln -s ../../redist/$THISFILE sdk/demo/.
done
