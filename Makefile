CXXFLAGS += -Wall -Wextra -Werror
#CXXFLAGS += -g
CXXFLAGS += -O2
LDLIBS   += -lboost_filesystem

CXXFLAGS += `pkg-config --cflags xmms2-client-cpp`
LDLIBS   += `pkg-config --libs   xmms2-client-cpp`


BINARIES = xmms2_jump-monitor xmms2_jump-makedb

ifndef WITHOUT_GTK
  BINARIES += xmms2_jump-query-gtk
  xmms2_jump-query-gtk: LDLIBS += `pkg-config --libs   gtkmm-2.4`
  gtkfe.o: CXXFLAGS += `pkg-config --cflags gtkmm-2.4`
endif
ifndef WITHOUT_CURSES
  BINARIES += xmms2_jump-query-curses
  xmms2_jump-query-curses: LDLIBS += -lncurses
endif


all: $(BINARIES)

xmms2_jump-monitor: xmms2_jump-monitor.o common.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
xmms2_jump-makedb: xmms2_jump-makedb.o jumpdb.o common.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
xmms2_jump-query-%: %fe.o frontend.o xmms2_jump-query.o jumpdb.o common.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@


common.o: common.cc common.h
cursesfe.o: cursesfe.cc common.h frontend.h
frontend.o: frontend.cc frontend.h common.h index_searcher.h jumpdb.h
gtkfe.o: gtkfe.cc common.h frontend.h
jumpdb.o: jumpdb.cc jumpdb.h common.h
xmms2_jump-makedb.o: xmms2_jump-makedb.cc common.h jumpdb.h frontend.h
xmms2_jump-monitor.o: xmms2_jump-monitor.cc common.h
xmms2_jump-query.o: xmms2_jump-query.cc common.h index_searcher.h \
  jumpdb.h frontend.h


clean:
	$(RM) $(BINARIES) *.o core core.* *~

.PHONY: all clean
