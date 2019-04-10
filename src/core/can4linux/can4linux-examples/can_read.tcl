#!/usr/bin/tclsh
# Tcl CAN Layer 2 example
# simple CAN Analyzer
#

load ./canLtwo.so
puts "....... swig wrapper loaded"

# open the can interface /dev/can0
# but before, set the baud rate if other than default
exec /bin/echo 125 > /proc/sys/dev/Can/Baud
puts "....... bit rate changed"

# use /dev/can0 as default device
set device 0
# but in case one argument is given as <number>
# or can<number>
if { $argc >= 1 } {
	set device [lindex $argv 0]
	if { [string equal -length 3 "can" $device] } {
	    set device  [string range $device 3 end]
	}
}

set can_fd [can_open $device]
puts "....... /dev/can$device opened, got descriptor $can_fd"

puts "... wait for messages"
# now go into receive loop
#
while 1 {
    puts [can_read2 $can_fd 0]
}
can_close $can_fd
puts "....... /dev/can$device closed"

