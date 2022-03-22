# Copyright (c) 2018 by Thomas A. Early N7TAE

# if you change these locations, make sure the sgs.service file is updated!
BINDIR=/usr/local/bin

# choose this if you want debugging help
#CPPFLAGS=-g -ggdb -W -Wall
# or, you can choose this for a much smaller executable without debugging help
CFLAGS=-W -Wall

tsdspacket :  tsdspacket.c QnetTypeDefs.h
	g++ $(CPPFLAGS) -o tsdspacket tsdspacket.c

clean:
	$(RM) *.o tsdspacket

# install, uninstall need root priviledges
install : tsdspacket
	$(CP) -f tsdspacket $(BINDIR)

uninstall :
	$(RM) -f $(BINDIR)/tsdspacket
