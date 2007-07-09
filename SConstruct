env = Environment()

env.Append(CXXFLAGS = '-Wall -Wextra -Werror')
#env.Append(CXXFLAGS = '-g')
env.Append(CXXFLAGS = '-O2')
env.Append(LIBS = ['boost_filesystem'])
#env.Append(CXXFLAGS = '-O2 -pg')
#env.Append(LINKFLAGS = '-O2 -pg')
#env.Append(LIBS = ['profiler'])

env.ParseConfig('pkg-config --cflags --libs xmms2-client-cpp')

gtkmm_env = env.Copy()
gtkmm_env.ParseConfig('pkg-config --cflags --libs gtkmm-2.4')
ncurs_env = env.Copy()
ncurs_env.Append(LIBS = ['ncurses'])

common   = env.Object("common.cc")
jumpdb   = env.Object("jumpdb.cc")
frontend = env.Object("frontend.cc")
query    = env.Object("xmms2_jump-query.cc")


env.Program(
	target = "xmms2_jump-monitor",
	source = [
		"xmms2_jump-monitor.cc",
		common,
	],
)

env.Program(
	target = "xmms2_jump-makedb",
	source = [
		"xmms2_jump-makedb.cc",
		common,
		jumpdb,
	],
)

gtkmm_env.Program(
	target = "xmms2_jump-query-gtk",
	source = [
		"gtkfe.cc",
		query,
		common,
		jumpdb,
		frontend,
	],
)

ncurs_env.Program(
	target = "xmms2_jump-query-curses",
	source = [
		"cursesfe.cc",
		query,
		common,
		jumpdb,
		frontend,
	],
)
