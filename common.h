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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <string>
typedef std::string const& strref;

#define CLIENT_NAME "xmms2_jump"
#define DATA_DIR_NAME "xmms2_jump"
#define NAMES_FILE_NAME "names.dat"
#define INDEX_FILE_NAME "index.dat"

#include <boost/filesystem/path.hpp>
boost::filesystem::path cache_path();

#endif // __COMMON_H__
