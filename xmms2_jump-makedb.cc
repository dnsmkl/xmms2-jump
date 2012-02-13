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
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <ios>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <xmmsclient/xmmsclient++.h>
#include "common.h"
#include "jumpdb.h"
#include "frontend.h"

using std::string;
using std::vector;
using std::exit;
using std::cerr;
using std::endl;


const char *xmms_usercachedir_get (char *buf, int len);


// A little string algorithm to remove redundant spaces.
// If item is "T", then this basically does s/T+/T/g.
template <typename ForwardIterator, typename OutputIterator, typename T>
ForwardIterator compress_copies(ForwardIterator first,
                                ForwardIterator last,
                                OutputIterator result,
                                T item) {
	ForwardIterator& src = first;
	ForwardIterator& dst = result;
	bool matched = false;

	while (src != last) {
		if (*src != item) {
			*dst = *src;
			matched = false;
			++src; ++dst;
		} else if (!matched) {
			*dst = *src;
			matched = true;
			++src; ++dst;
		} else {
			++src;
		}
	}

	return dst;
}

int HexDigitToInt(char ch) {
	if (isdigit(ch)) return ch - '0';
	if (islower(ch)) return ch - 'a' + 10;
	if (isupper(ch)) return ch - 'A' + 10;
	return 0;
}

char EscapeToChar(char ch1, char ch2) {
	return (char)(16*HexDigitToInt(ch1) + HexDigitToInt(ch2));
}

string CleanURL(strref url) {
	string::size_type start = url.rfind('/');
	if (start == string::npos) start = 0;
	start += 1;

	string::size_type end = url.rfind('.');
	if (end == string::npos) end = url.length();
	else if (end <= start) end = url.length();

	string cleaned;

	for ( string::size_type pos = start ; pos < end ; ++pos ) {
		char ch = url[pos];
		if (ch == '+') { cleaned += ' '; continue; }
		if (ch != '%') { cleaned += ch;  continue; }
		if (pos + 2 >= end) break;
		char escape = EscapeToChar(url[pos+1], url[pos+2]);
		if (escape != '\0') cleaned += escape;
		pos += 2;
	}

	return cleaned;
}

// Some functors for cleaning the song names.
struct IsKeyChar : public std::unary_function<char, bool> {
	bool operator()(char c) {
		return
			std::islower(c) ||
			std::isdigit(c) ||
			c == '-' || c == ' ';
	}
};

struct IsSpaceChar : public std::unary_function<char, bool> {
	bool operator()(char c) {
		return c == '-';
	}
};


class IndexBuilder {
public:
	IndexBuilder() : client_(CLIENT_NAME) {}

	void Load();
	void Dump(std::ostream& names_file, std::ostream& index_file);

private:
	string FetchName(unsigned int id);
	string ExtractKey(strref name);

	Xmms::Client client_;
	vector<string> names_;
	JumpDB jumpdb_;
};

void IndexBuilder::Load() {
	client_.connect(std::getenv("XMMS_PATH"));
	Xmms::List<int> entry_list = client_.playlist.listEntries();
	int position = 0;
	for (Xmms::List<int>::const_iterator entry = entry_list.begin();
			entry != entry_list.end(); entry++ ) {
		string name = FetchName(*entry);
		string key = ExtractKey(name);
		names_.push_back(name);
		jumpdb_.AddEntry(position, key);
		position++;
	}
}

string IndexBuilder::FetchName(unsigned int id) {
	Xmms::Dict info = client_.medialib.getInfo(id);

	string artist, title;
	try { artist = info.get<string>("artist"); }
	catch ( Xmms::no_such_key_error& err ) {}

	try { title = info.get<string>("title"); }
	catch ( Xmms::no_such_key_error& err ) {}

	if (artist.empty() && title.empty()) {
		try {
			string url = info.get<string>("URL");
			string cleaned = CleanURL(url);
			//std::cerr << url << " --> " << cleaned << std::endl;
			return cleaned;
		}
		catch ( Xmms::no_such_key_error& err ) {
			return "";
		}
	} else if (artist.empty()) {
		return title;
	} else if (title.empty()) {
		return artist;
	} else {
		return artist + " - " + title;
	}
}

string IndexBuilder::ExtractKey(strref name) {
	string key = name;

	// Convert to lower case.
	// Gcc appears to inline tolower.
	std::transform(key.begin(), key.end(), key.begin(), tolower);
	// Convert certain characters to spaces.
	std::replace_if(key.begin(), key.end(), IsSpaceChar(), ' ');

	// Remove all non-key characters.
	key.erase(
			std::remove_if(key.begin(), key.end(),
				!boost::bind(IsKeyChar(), _1)),
			key.end());
	key.erase(
			compress_copies(key.begin(), key.end(), key.begin(), ' '),
			key.end());

	// FIXME: Do leading and trailing spaces more efficiently.
	if (*key.begin()  == ' ') key.erase(key.begin() );
	if (*key.rbegin() == ' ') key.erase(key.end()-1 );

	return key;
}

void IndexBuilder::Dump(std::ostream& names_file, std::ostream& index_file) {
	if (!names_file || !index_file)
		throw std::runtime_error("Couldn't open output files.");

	for ( vector<string>::iterator it = names_.begin() ;
			it != names_.end() ; ++it ) {
		names_file.write(it->c_str(), it->length() + 1);
	}

	jumpdb_.Dump(index_file);
}

int main(int argc, char *argv[]) {
	argc = argc;
	argv = argv;

	IndexBuilder builder;

	try {
		builder.Load();
	}
	catch ( Xmms::connection_error& err ) {
		cerr << "Connection failed: " << err.what() << endl;
		exit(EXIT_FAILURE);
	}
	catch ( Xmms::result_error& err ) {
		cerr << "Error when asking for the playlist: " << err.what()
		     << endl;
		exit(EXIT_FAILURE);
	}

	boost::filesystem::path cpath = cache_path();
	boost::filesystem::create_directories(cpath);
	std::ofstream names_file((cpath/NAMES_FILE_NAME).string().c_str());
	std::ofstream index_file((cpath/INDEX_FILE_NAME).string().c_str());

	builder.Dump(names_file, index_file);

	return 0;
}
