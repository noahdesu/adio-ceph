#!/bin/sh
#
# $HEADER$
#
# This script is used to make a nightly snapshot tarball of Open MPI.
#
# $1: scratch root
# $2: e-mail address for destination
# $3: SVN root
# $4: dest dir
#

scratch_root="$1"
email="$2"
svnroot="$3"
destdir="$4"

# Set this to any value for additional output; typically only when
# debugging
debug=

start_time="`date`"

# Sanity checks
if test -z "$scratch_root" -o -z "$email" -o -z "$svnroot" \
    -o -z "$destdir"; then
    echo "Must specify scratch root directory, e-mail address, and SVN root"
    exit 1
fi

# send a mail
send_mail() {
    print "sending mail to $email"
}

# do the work
do_command() {
    cmd="$*"
    logfile="$scratch_root/logs/20-command.txt"
    rm -f "$logfile"
    if test -n "$debug"; then
        echo "*** Running command: $cmd"
        eval $cmd 2>&1 | tee "$logfile"
        st=$?
        echo "*** Command complete: exit status: $st"
    else
        eval $cmd > "$logfile" 2>&1
        st=$?
    fi
    if test "$st" != "0"; then
        cat > "$scratch_root/logs/15-error.txt" <<EOF

ERROR: Command returned a non-zero exist status
       $cmd

Start time: $start_time
End time:   `date`

=======================================================================
EOF
        cat > "$scratch_root/logs/25-error.txt" <<EOF
=======================================================================

Your friendly daemon,
Cyrador
EOF
        send_mail
        exit 2
    fi
    rm -f "$logfile"
}

# see if the destination directory exists
if test ! -d "$destdir"; then
    mkdir -p "$destdir"
fi
if test ! -d "$destdir"; then
    echo "Could not cd to dest dir: $destdir"
    exit 1
fi

# move into the scratch directory
if test ! -d "$scratch_root"; then
    mkdir -p "$scratch_root"
fi
if test ! -d "$scratch_root"; then
    echo "Could not cd to scratch root: $scratch_root"
    exit 1
fi
cd "$scratch_root"

# cleanup old files before we start
rm -rf ompi logs
mkdir logs

# start up the log file
logfile="$scratch_root/logs/10-announce.txt"
cat > $logfile <<EOF
Building the nightly snapshot SVN tarball

Start: `date`
----------------------------------------------------
EOF

# checkout a clean version
do_command "svn co $svnroot ompi"

# lets work on it
cd ompi
svnversion="`svnversion .`"

# autogen is our friend
do_command "./autogen.sh"

# do config
do_command "./configure --enable-dist"

# do make dist
do_command "make dist"

# move the resulting tarballs to the destdir
gz="`/bin/ls openmpi*tar.gz`"
bz2="`/bin/ls openmpi*tar.bz2`"
mv $gz $bz2 $destdir
cd $destdir
rm -f openmpi-latest.tar.gz openmpi-latest.tar.bz2
ln -s $gz openmpi-latest.tar.gz
ln -s $bz2 openmpi-latest.tar.bz2

# generate md5 and sha1 sums
rm -f md5sums.txt sha1sums.txt
touch md5sums.txt sha1sums.txt
for file in `/bin/ls *gz *bz2 | grep -v latest`; do
    md5sum $file >> md5sums.txt
    sha1sum $file >> sha1sums.txt
done

# send success mail
Mail -s "Success" "$email" <<EOF
Creating nightly snapshot SVN tarball a success.

Start time: $start_time
End time:   `date`

Your friendly daemon,
Cyrador
EOF
