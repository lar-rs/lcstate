[![Build Status](https://gitlab.gnome.org/GNOME/gnome-control-center/badges/master/build.svg)](https://gitlab.gnome.org/lar/gnome-control-center/pipelines)
[![Coverage report](https://gitlab.gnome.org/GNOME/gnome-control-center/badges/master/coverage.svg)](https://gnome.pages.gitlab.gnome.org/gnome-control-center/)
[![License](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://gitlab.gnome.org/GNOME/gnome-control-center/blob/master/COPYING)

# wqm-candbus

wqm can dbus interface hal

## Contributing

Contributing link or full text

## Testing Unstable Settings

## Reporting Bugs

Bugs should be reported to the bug tracking system under the product
gnome-control-center. It is available at

- [Azure Issues](https://gitlab.gnome.org/GNOME/gnome-control-center/issues).
- [GitLab Issues]().
- [GitHub Issues]().

In the report please include the following information:

 * Operating system and version
 * For Linux, version of the C library
 * Exact error message
 * Steps to reproduce the bug
 * If the bug is a visual defect, attach a screenshot
 * If the bug is a crash, attach a backtrace if possible [see below]

### How to get a backtrace

If the crash is reproducible, follow the steps to obtain a 
backtrace:

Install debug symbols for c .

Run the program in gdb [the GNU debugger] or any other debugger.

    gdb gnome-control-center

Start the program.

    (gdb) run

Reproduce the crash and when the program exits to (gdb) prompt, get the backtrace.

    (gdb) bt full

Once you have the backtrace, copy and paste it into the 'Comments' field or attach it as
a file to the bug report.
