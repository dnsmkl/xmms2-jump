env = Environment()

env.Append(CXXFLAGS = '-Wall -Wextra -Werror')
env.Append(CXXFLAGS = '-g')
env.Append(LIBS = ['boost_filesystem'])
#env.Append(CXXFLAGS = '-O2')
#env.Append(CXXFLAGS = '-g')
#env.Append(LINKFLAGS = '-O2 -pg')
#env.Append(LIBS = ['profiler'])

env.ParseConfig('pkg-config --cflags --libs xmms2-client-cpp gtkmm-2.4')


env.Program(
	target = "xmms2_jump-monitor",
	source = [
		"xmms2_jump-monitor.cc",
		"common.cc",
	],
)

env.Program(
	target = "xmms2_jump-makedb",
	source = [
		"xmms2_jump-makedb.cc",
		"common.cc",
		"jumpdb.cc",
	],
)

env.Program(
	target = "xmms2_jump-query",
	source = [
		"xmms2_jump-query.cc",
		"common.cc",
		"jumpdb.cc",
		"frontend.cc",
		"gtkfe.cc",
	],
)
