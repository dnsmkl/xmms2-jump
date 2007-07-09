/*
 * xmms2_jump: Quickly search your XMMS2 playlist.
 * Copyright (C) 2007  David Scott Reiss
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "common.h"
#include <cstdlib>
#include <iostream>

using std::cerr;
using std::endl;
using boost::filesystem::path;

boost::filesystem::path cache_path() {
	path cpath;
	char *home, *xdg_cache_home; ;
	if ((xdg_cache_home = std::getenv("XDG_CACHE_HOME")) != NULL) {
		cpath = xdg_cache_home;
	} else if ((home = std::getenv("HOME")) != NULL) {
		cpath = home;
		// TODO: Might not work with Boost 1.34
		cpath /= path(".cache", boost::filesystem::native);
	} else {
		cerr << "You must define XDG_CACHE_HOME or at least HOME." << endl;
		std::exit(EXIT_FAILURE);
	}
	cpath /= DATA_DIR_NAME;
	return cpath;
}
