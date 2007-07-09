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

#include "frontend.h"
#include <boost/algorithm/string.hpp>
#include "index_searcher.h"

void Frontend::ItemSelected(int position) {
	searcher_->ItemSelected(position);
}

bool Frontend::IsCharAppend(strref text) {
	return text.size() == last_text_.size()+1
		&& boost::starts_with(text, last_text_);
}

const std::vector<int>& Frontend::Search(strref text) {
	bool is_append = IsCharAppend(text);

	last_text_ = text;

	if (is_append) {
		return searcher_->PushChar(text[text.size()-1]);
	} else {
		return searcher_->Search(text);
	}
}

const std::vector<int>& Frontend::AddChar(char ch) {
	last_text_ += ch;
	return searcher_->PushChar(ch);
}

const std::vector<int>& Frontend::PopChar() {
	last_text_.erase(last_text_.length() - 1);
	return searcher_->Search(last_text_);
}

void Frontend::SetSearcher(IndexSearcher* searcher) {
	searcher_ = searcher;
}
