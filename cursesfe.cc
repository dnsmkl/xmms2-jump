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
	CursesGui() : xpos_(0), ypos_(2) {}

	virtual void AddEntry(int position, strref name);
	virtual void Display();

private:
	void InitCurses();
	void SetListSize();
	void AdjustNames();
	void UpdateCount();
	void DisplayNames();


	unordered_map<int,string> names_;
	vector<int> const* survivors_;
	string list_size_;
	int xpos_, ypos_;  // XXX rename, ace.
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

		int c = mvgetch(0, xpos_);
		//endwin();
		//std::cerr << c << std::endl;
		//return;
		if ( c < 0x100 && std::isprint(c)) {
			xpos_++;
			addch(c);
			survivors_ = &(AddChar((char)c));
			continue;
		}

		switch (c) {
			case KEY_BACKSPACE:
			case KEY_DC:
			case 0x7f:
				if (xpos_ > 0) {
					xpos_--;
					move(0, xpos_);
					delch();
					survivors_ = &(PopChar());
				}
			break;
			case KEY_DOWN:
				ypos_++;
				DisplayNames();  // XXX be more efficient.
			break;
			case KEY_UP:
				ypos_--;
				if (ypos_ < 2) ypos_ = 2;
				DisplayNames();  // XXX be more efficient.
			break;
			case KEY_ENTER:
			case '\r':
			case '\n':
				endwin();
				ItemSelected(survivors_->at(ypos_-2));
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
	int ypos = 2;
	for ( vector<int>::const_iterator it = survivors_->begin() ;
			it != survivors_->end() ; ++it ) {
		if (ypos == LINES) break;

		if (ypos == ypos_) standout();
		mvaddstr(ypos, 0, names_[*it].c_str());
		if (ypos == ypos_) standend();

		++ypos;
	}

	if (ypos != LINES) {
		move(ypos, 0);
		clrtobot();
	}
}


Frontend* Frontend::get() {
	return new CursesGui();
}
