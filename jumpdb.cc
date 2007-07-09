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

#include "jumpdb.h"
#include <string>
#include <iterator>
#include <algorithm>
#include <iostream>

using std::string;
using std::vector;

void JumpDB::AddEntry(int position, strref lookup_key) {
	for ( string::size_type glen = 0 ; glen <= lookup_key.length(); ++glen ) {
		for ( string::size_type spos = 0 ; spos + glen <= lookup_key.length() ; ++spos ) {
			// FIXME: Use StringPiece.
			// FIXME: Only check last position if this is not the first gram.
			string gram = lookup_key.substr(spos, glen);
			string::size_type space = gram.rfind(' ');
			if (space != string::npos) {
				spos += space;
				continue;
			}
			std::vector<int>& vec = ngram_map_[gram];
			if (vec.empty() || vec.back() != position)
				vec.push_back(position);
		}
	}
}

/*
void JumpDB::Prepare() {
}
*/

void JumpDB::Dump(std::ostream& outfile) {
	Map::size_type msize = ngram_map_.size();
	outfile.write((char*)&msize, sizeof(msize));

	for ( Map::iterator mit = ngram_map_.begin() ;
			mit != ngram_map_.end() ; ++mit ) {

		outfile.write(mit->first.c_str(), mit->first.length() + 1);

		vector<int>::size_type vsize = mit->second.size();
		outfile.write((char*)&vsize, sizeof(vsize));
		outfile.write((char*)&mit->second[0], vsize * sizeof(int));
	}
}

void JumpDB::Load(std::istream& infile) {
	Map::size_type msize = ngram_map_.size();
	infile.read((char*)&msize, sizeof(msize));
	ngram_map_.rehash((int)(msize * ngram_map_.max_load_factor() + 2));

	for ( ; msize > 0 ; --msize) {
		string word;
		std::getline(infile, word, '\0');

		vector<int>::size_type vsize;
		infile.read((char*)&vsize, sizeof(vsize));

		std::vector<int>& vec = ngram_map_[word];

		vec.resize(vsize);
		infile.read((char*)&vec[0], vsize * sizeof(int));
	}
}

void JumpDB::Query::CullWord(bool is_first) {
	//std::cerr << is_first << ' ' << word_ << std::endl;
	if (is_first) {
		survivors_ = owner_.ngram_map_[word_];
	} else {
		std::vector<int>& wordvec = owner_.ngram_map_[word_];
		std::vector<int> intersection;
		std::set_intersection(survivors_.begin(), survivors_.end(),
				wordvec.begin(), wordvec.end(),
				std::back_inserter(intersection));
		survivors_.swap(intersection);
	}
}

void JumpDB::Query::AddChar(char c) {
	if (c == ' ') {
		word_.clear();
		return;
	}

	word_ += c;
	CullWord(false);
}

void JumpDB::Query::Search(strref query) {
	if (query.empty()) {
		survivors_ = owner_.ngram_map_[""];
		word_.clear();
		return;
	}

	bool first_word = true;
	word_.clear();

	// Can't be end because it is not empty.
	string::const_iterator it = query.begin();

	while (it != query.end()) {
		char ch = *it;
		++it;

		if (ch == ' ') {
			CullWord(first_word);
			first_word = false;
			word_.clear();
			continue;
		}

		word_ += ch;
		
		if (it == query.end()) {
			CullWord(first_word);
		}
	}
}

/*
void JumpDB::Query::PopChar() {
}
*/
