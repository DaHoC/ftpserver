CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g
#$(shell root-config --cflags)
# -Wall
LDFLAGS=-g
#$(shell root-config --ldflags)
LDLIBS=
#$(shell root-config --libs)

SRCS=servercore.cpp serverconnection.cpp fileoperator.cpp ftpserver.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: ftpserver

ftpserver: $(OBJS)
	$(CXX) $(LDFLAGS) -o ftpserver $(OBJS) $(LDLIBS)

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean: 
	$(RM) $(OBJS) ftpserver

distclean: clean
	$(RM) *~ .depend

include .depend
