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

#ifndef __JUMPDB_H__
#define __JUMPDB_H__

#include <cctype>
#include <string>
#include <vector>
#include <tr1/unordered_map>
#include <iostream>
#include "common.h"

class JumpDB {

public:
	class Query;

	void AddEntry(int position, strref lookup_key);
	void Dump(std::ostream& outfile);
	void Load(std::istream& infile);

	//void Prepare();

	class Query {
	public:
		Query(JumpDB& owner)
			: owner_(owner)
			, survivors_(owner_.ngram_map_[""])
		{}

		void AddChar(char c);
		void Search(strref query);
		//void PopChar();
		const std::vector<int>& survivors() {
			return survivors_;
		}

	private:
		void CullWord(bool is_first);

		JumpDB& owner_;
		std::vector<int> survivors_;
		std::string word_;
	};

private:
	typedef std::tr1::unordered_map< std::string , std::vector<int> > Map;
	Map ngram_map_;
};

#endif // __JUMPDB_H__
