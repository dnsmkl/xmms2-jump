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

#ifndef __INDEX_SEARCHER_H__
#define __INDEX_SEARCHER_H__

#include <vector>
#include <string>
#include <iostream>
#include <boost/scoped_ptr.hpp>
#include <xmmsclient/xmmsclient++.h>
#include "common.h"
#include "jumpdb.h"

class Frontend;

// Main class that handles most of the logic of the program.
class IndexSearcher {
public:
	IndexSearcher(Frontend& frontend)
		: client_(CLIENT_NAME)
		, frontend_(frontend)
	{}

	// Functions called by main.
	// void Load();
	void Load(std::istream& names_file, std::istream& index_file);
	void Run();

	// Functions called by the frontend.
	void ItemSelected(int position);
	const std::vector<int>& Search(strref text);
	const std::vector<int>& PushChar(char ch);

private:
	std::string FetchName(unsigned int id);
	std::string ExtractKey(strref name);

	Xmms::Client client_;
	JumpDB jumpdb_;
	Frontend& frontend_;
	boost::scoped_ptr<JumpDB::Query> query_;
};

#endif // __INDEX_SEARCHER_H__
