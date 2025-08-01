# This file is meant to change some aspect of how weenix is built or run.

# Variables in this file should meet the following criteria:
# * They change some behavior in the building or running of weenix that someone
#   using weenix for educational purposes could reasonably want to change on a regular
#   basis. Note that variables like CFLAGS are not defined here because they should
#   generally not be changed.

#
# Setting any of these variables will control which parts of the source tree
# are built. To enable something set it to 1, otherwise set it to 0.
#
     DRIVERS=1
         VFS=0
        S5FS=0
          VM=0
     DYNAMIC=0
# When you finish S5FS, first enable "VM"; once this is working, then enable
# "DYNAMIC".

# Debug message behaviour: Edit `INIT_DBG_MODES` in kernel/util/debug.c to set
# which messages are shown.

# Switches for non-required components. If you wish to try implementing
# some extra features in Weenix, there are some pre-designed features
# you can add. Turn on one of these flags and re-compile Weenix. Please
# see the Wiki for details on what is provided by changing these flags
# and what you will need to implement to complete them, of course you
# are always free to implement your own features as well. Remember, though
# these features are not "extra-credit" they are purely for academic
# interest. The most important thing is that you have a working core
# implementation, and that is what you will be graded on. If you decide
# to implement extra features please make sure your core Weenix is working
# first, and make sure to make a copy of your working Weenix before you
# go breaking it, which we promise you will happen.

         SHADOWD=0 # shadow page cleanup
        MOUNTING=0 # be able to mount multiple file systems
          GETCWD=0 # getcwd(3) syscall-like functionality
        UPREEMPT=0 # userland preemption
        KPREEMPT=0 # kernel space preemption
             MTP=0 # multiple kernel threads per process
           PIPES=0 # pipe(2) functionality
          VGABUF=0 # Use a rudimentary VGA buffers instead of VT support.
	KPREEMPT=0
        RENAMEDIR=0

# Set the number of terminals that we should be launching.
        NTERMS=3

# Set the number of disks that we should be launching with
        NDISKS=1

# terminal binary to use when opening a second terminal for gdb
        GDB_TERM=xterm
        GDB_PORT=1234

# The amount of physical memory which will be available to Weenix (in megabytes)
# XXX MEMORY=256 is hardcoded in ./weenix right now -- this line here is
#     currently ignored
        MEMORY=256

# Parameters for the hard disk we build (must be compatible!)
# If the FS is too big for the disk, BAD things happen!
        DISK_BLOCKS=2048 # For fsmaker
        DISK_INODES=240  # For fsmaker

# Boolean options specified in this specified in this file that should be
# included as definitions at compile time
        COMPILE_CONFIG_BOOLS=" DRIVERS VFS S5FS VM FI DYNAMIC MOUNTING MTP GETCWD RENAMEDIR UPREEMPT PIPES KPREEMPT"
# As above, but not booleans
        COMPILE_CONFIG_DEFS=" NTERMS NDISKS DBG DISK_SIZE "
