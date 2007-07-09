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

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <tr1/unordered_map>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <signal.h>
#include <curses.h>
#include "common.h"
#include "frontend.h"

using std::string;
using std::pair;
using std::vector;
using std::tr1::unordered_map;
using boost::lexical_cast;


static void sigint(int sig) {
	sig = sig;
    (void) endwin();
	std::exit(EXIT_SUCCESS);
}


class CursesGui : public Frontend
{
public:
	CursesGui()
		: cursor_pos_(0)
		, select_offset_(0)
		, screen_offset_(0)
	{}

	virtual void AddEntry(int position, strref name);
	virtual void Display();

private:
	void InitCurses();
	void SetListSize();
	void AdjustNames();
	void UpdateCount();
	void DisplayNames();

	static const int header_size = 2;

	unordered_map<int,string> names_;
	vector<int> const* survivors_;
	string list_size_;
	int cursor_pos_;
	int select_offset_;
	int screen_offset_;
};


void CursesGui::AddEntry(int position, strref name) {
	names_[position] = name;
}

void CursesGui::Display() {
	survivors_ = &(Search(""));
	if (survivors_->size() != names_.size())
		throw std::runtime_error("Size mismatch.");

	InitCurses();
	SetListSize();
	AdjustNames();



	while (true) {
		UpdateCount();
		DisplayNames();
		// TODO: Avoid full DisplayNames when we just move the selection.
		// Unfortunately, it is so darn fast as is that I'm not motivated.

		int c = mvgetch(0, cursor_pos_);
		//endwin();
		//std::cerr << c << std::endl;
		//return;
		if ( c < 0x100 && std::isprint(c)) {
			cursor_pos_++;
			addch(c);
			survivors_ = &(AddChar((char)c));
			select_offset_ = 0;
			screen_offset_ = 0;
			continue;
		}

		switch (c) {
			case KEY_BACKSPACE:
			case KEY_DC:
			case 0x7f:
				if (cursor_pos_ > 0) {
					cursor_pos_--;
					move(0, cursor_pos_);
					delch();
					survivors_ = &(PopChar());
					select_offset_ = 0;
					screen_offset_ = 0;
				}
			break;
			case KEY_DOWN:
				select_offset_++;
				if (select_offset_ >= (int)survivors_->size())
					select_offset_ = survivors_->size() - 1;
				if (select_offset_ - screen_offset_ >= LINES - header_size)
					screen_offset_++;
			break;
			case KEY_UP:
				select_offset_--;
				if (select_offset_ < 0)
					select_offset_ = 0;
				if (select_offset_ < screen_offset_)
					screen_offset_--;
			break;
			case KEY_NPAGE /* Page Down*/ :
				select_offset_ += LINES - header_size - 1;
				if (select_offset_ >= (int)survivors_->size())
					select_offset_ = survivors_->size() - 1;
				if (select_offset_ - screen_offset_ >= LINES - header_size)
					screen_offset_ = select_offset_ - LINES + header_size + 1;
			break;
			case KEY_PPAGE /* Page Up */:
				select_offset_ -= LINES - header_size - 1;
				if (select_offset_ < 0)
					select_offset_ = 0;
				if (select_offset_ < screen_offset_)
					screen_offset_ = select_offset_;
			break;
			case KEY_ENTER:
			case '\r':
			case '\n':
				endwin();
				ItemSelected(survivors_->at(select_offset_));
				return;
			case '\033' /* ESC */:
				endwin();
				return;
		}
	}
}

void CursesGui::InitCurses() {
	(void) signal(SIGINT, sigint);  /* arrange interrupts to terminate */
    (void) initscr();               /* initialize the curses library */
    (void) keypad(stdscr, TRUE);    /* enable keyboard mapping */
    (void) nonl();                  /* don't do do NL->CR/NL on output */
    (void) cbreak();                /* don't wait for \n */
    (void) noecho();                /* do not echo input  */

	string dashes(COLS, '-');
	mvaddstr(1, 0, dashes.c_str());
}

void CursesGui::SetListSize() {
	list_size_ = lexical_cast<string>(names_.size());
}

void CursesGui::AdjustNames() {
	int max_len = lexical_cast<string>(names_.size()).length();
	for ( unordered_map<int,string>::iterator it = names_.begin() ;
			it != names_.end() ; ++it ) {
		string pos = lexical_cast<string>(it->first);
		string spaces(max_len - pos.length(), ' ');
		string prefix = spaces + pos + ' ';
		if (COLS - prefix.length() < it->second.length())
			it->second.erase(COLS - prefix.length());
		it->second.replace(0, 0, prefix);
		while (it->second.length() < (string::size_type)COLS)
			it->second += ' ';
	}
}

void CursesGui::UpdateCount() {
	string surv = lexical_cast<string>(survivors_->size());
	string dashes(list_size_.length() - surv.length() + 3, '-');
	string display = dashes + ' ' + surv + '/' + list_size_ + ' ';
	mvaddstr(1, 0, display.c_str());
}

void CursesGui::DisplayNames() {
	int yoff = 0;

	assert(screen_offset_ <= (int)survivors_->size());

	// Skip over everything that is scrolled offscreen.
	for ( vector<int>::const_iterator it = survivors_->begin()
			+ screen_offset_ ; it != survivors_->end() ; ++it ) {
		if (yoff + header_size == LINES) break;

		if (yoff == select_offset_ - screen_offset_) standout();
		mvaddstr(yoff + header_size, 0, names_[*it].c_str());
		if (yoff == select_offset_ - screen_offset_) standend();

		++yoff;
	}

	if (yoff + header_size != LINES) {
		move(yoff + header_size, 0);
		clrtobot();
	}
}


Frontend* Frontend::get() {
	return new CursesGui();
}
