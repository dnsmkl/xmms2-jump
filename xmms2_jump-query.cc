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

#include <cstdlib>
#include <cctype>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <unistd.h>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <xmmsclient/xmmsclient++.h>
#include "common.h"
#include "index_searcher.h"
#include "jumpdb.h"
#include "frontend.h"

using std::string;
using std::vector;
using std::exit;
using std::cout;
using std::endl;



void IndexSearcher::Load(std::istream& names_file, std::istream& index_file) {
	if (!names_file || !index_file)
		throw std::runtime_error("Couldn't open index files.");

	int position = 0;
	while (true) {
		string name;
		std::getline(names_file, name, '\0');
		if (names_file.eof()) break;
		frontend_.AddEntry(position, name);
		position++;
	}

	jumpdb_.Load(index_file);
}

void IndexSearcher::Run() {
	//jumpdb_.Prepare();
	query_.reset(new JumpDB::Query(jumpdb_));
	frontend_.Display();
}

void IndexSearcher::ItemSelected(int position) {
	try {
		client_.connect(std::getenv("XMMS_PATH"));
		client_.playlist.setNext(position);
		client_.playback.tickle();
	} catch (std::exception& e) {
		std::cerr << "Error selecting track: "
		          << e.what() << std::endl;
	}
}

const vector<int>& IndexSearcher::Search(strref text) {
	query_.reset(new JumpDB::Query(jumpdb_));
	query_->Search(text);
	return query_->survivors();
}

const vector<int>& IndexSearcher::PushChar(char ch) {
	query_->AddChar(ch);
	return query_->survivors();
}

int main() {

	boost::scoped_ptr<Frontend> fe(Frontend::get());
	IndexSearcher searcher(*fe);
	fe->SetSearcher(&searcher);

	boost::filesystem::path cpath = cache_path();
	std::ifstream names_file((cpath/NAMES_FILE_NAME).string().c_str());
	std::ifstream index_file((cpath/INDEX_FILE_NAME).string().c_str());

	searcher.Load(names_file, index_file);

	searcher.Run();

	return 0;
}
