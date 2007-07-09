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
#include <iostream>
#include <tr1/functional>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <xmmsclient/xmmsclient++.h>
#include "common.h"

using std::string;
using boost::filesystem::path;


class PlaylistMonitor {
public:
	PlaylistMonitor(path const& names_path, path const& index_path)
		: names_path_(names_path)
		, index_path_(index_path)
		, client_(CLIENT_NAME)
	{}

	void Run();

private:
	void Disconnected();
	bool ErrorHandler(strref error);
	bool PlaylistLoaded(strref playlist);
	bool PlaylistChanged(Xmms::Dict const& update);

	void DeleteDatabase();

	path const& names_path_;
	path const& index_path_;
	string current_playlist_;
	Xmms::Client client_;
};

void PlaylistMonitor::Run() {
	client_.connect(std::getenv("XMMS_PATH"));

	client_.setDisconnectCallback(
			std::tr1::bind(
				std::tr1::mem_fn(&PlaylistMonitor::Disconnected),
				this));

	client_.playlist.broadcastChanged()(
			Xmms::bind(&PlaylistMonitor::PlaylistChanged, this),
			Xmms::bind(&PlaylistMonitor::ErrorHandler   , this));

	client_.playlist.broadcastLoaded()(
			Xmms::bind(&PlaylistMonitor::PlaylistLoaded, this),
			Xmms::bind(&PlaylistMonitor::ErrorHandler  , this));

	current_playlist_ = client_.playlist.currentActive();

	client_.getMainLoop().run();
}

void PlaylistMonitor::Disconnected() {
	std::cerr << "Disconnected." << std::endl;
	std::exit(EXIT_SUCCESS);
}

bool PlaylistMonitor::ErrorHandler(strref error) {
	std::cerr << "Xmms2 Error: " << error << std::endl;
	std::exit(EXIT_FAILURE);
	return false;
}

bool PlaylistMonitor::PlaylistLoaded(strref playlist) {
	current_playlist_ = playlist;
	DeleteDatabase();
	return true;
}

bool PlaylistMonitor::PlaylistChanged(Xmms::Dict const& update) {
	string playlist = update.get<string>("name");
	if (current_playlist_ == playlist)
		DeleteDatabase();
	return true;
}

void PlaylistMonitor::DeleteDatabase() {
	//std::cerr << "Deleting database." << std::endl;
	boost::filesystem::remove(names_path_);
	boost::filesystem::remove(index_path_);
}


int main() {

	path cpath = cache_path();
	path names_path = (cpath/NAMES_FILE_NAME);
	path index_path = (cpath/INDEX_FILE_NAME);

	try {
		PlaylistMonitor mon(names_path, index_path);
		mon.Run();
	}
	catch( Xmms::connection_error& err ) {
		std::cerr << "Connection failed: " << err.what() << std::endl;
		std::exit(EXIT_FAILURE);
	}

	return 0;
}
