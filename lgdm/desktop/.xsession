#!/bin/sh
hsetroot -fill /usr/local/share/lar-background.png
xfwm4 &
onboard &
exec run-parts ~/.xinitrc.d
