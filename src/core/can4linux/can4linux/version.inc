# Version information for the can4linux driver
VERSION=4
REL=5
RELEASE=CAN4LINUX-$(VERSION)_$(REL)
SVNVERSION="SVN version $(shell svnversion -n $(M)/version.inc)"
# $(svn info | grep -i revision | cut -f2 -d: | tr -d '[:space:]')
DVERSION=$(VERSION).$(REL)
