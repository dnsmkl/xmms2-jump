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

#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/separator.h>
#include <gtkmm/button.h>
#include <gtkmm/stock.h>
#include "common.h"
#include "frontend.h"

using std::string;
using std::pair;
using std::vector;


class X2JWindow : public Frontend, public Gtk::Window
{
public:
	X2JWindow();
	~X2JWindow() {}

	virtual void AddEntry(int position, strref name);
	virtual void Display();

private:
	void Reset();

	vector< pair<int, string> > entries_;


	bool on_window_key_press(GdkEventKey* event);
	void on_queue();
	void on_jumpt();
	void on_close();
	void on_change();
	void on_activate_entry();
	void on_activate_treeview(
			const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);


	Gtk::Entry entry_;
	Gtk::ScrolledWindow scroll_;
	Gtk::TreeView tree_view_;
	Glib::RefPtr<Gtk::ListStore> tree_model_;
	Gtk::TreeModel::ColumnRecord col_rec_;
	Gtk::TreeModelColumn<int> pos_column_;
	Gtk::TreeModelColumn<Glib::ustring> name_column_;
};


X2JWindow::X2JWindow() {
	set_border_width(5);
	set_default_size(640, 480);

	this->signal_key_press_event().connect(
			sigc::mem_fun(*this, &X2JWindow::on_window_key_press));

	col_rec_.add(pos_column_);
	col_rec_.add(name_column_);
	tree_model_ = Gtk::ListStore::create(col_rec_);
	tree_view_.set_model(tree_model_);


	Gtk::VBox* main_box   = Gtk::manage(new Gtk::VBox(false, 5));

	Gtk::HSeparator* sep = Gtk::manage(new Gtk::HSeparator());

	Gtk::HBox* button_box = Gtk::manage(new Gtk::HBox(false, 5));
	Gtk::Button* b_queue = Gtk::manage(new Gtk::Button("Queue"));
	Gtk::Button* b_jumpt = Gtk::manage(new Gtk::Button(Gtk::Stock::JUMP_TO));
	Gtk::Button* b_close = Gtk::manage(new Gtk::Button(Gtk::Stock::CLOSE));


	entry_.signal_activate().connect(
			sigc::mem_fun(*this, &X2JWindow::on_activate_entry));
	entry_.signal_changed().connect(
			sigc::mem_fun(*this, &X2JWindow::on_change));

	tree_view_.signal_row_activated().connect(
			sigc::mem_fun(*this, &X2JWindow::on_activate_treeview));

	scroll_.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);

	tree_view_.set_headers_visible(false);
	tree_view_.append_column("", pos_column_);
	tree_view_.append_column("", name_column_);

	b_queue->signal_clicked().connect(
			sigc::mem_fun(*this, &X2JWindow::on_queue));
	b_jumpt->signal_clicked().connect(
			sigc::mem_fun(*this, &X2JWindow::on_jumpt));
	b_close->signal_clicked().connect(
			sigc::mem_fun(*this, &X2JWindow::on_close));


	add(*main_box);

	main_box->pack_start(entry_,      Gtk::PACK_SHRINK);
	main_box->pack_start(scroll_,     Gtk::PACK_EXPAND_WIDGET);
	main_box->pack_start(*sep,        Gtk::PACK_SHRINK);
	main_box->pack_start(*button_box, Gtk::PACK_SHRINK);

	scroll_.add(tree_view_);

	button_box->pack_end(*b_close, Gtk::PACK_SHRINK);
	button_box->pack_end(*b_jumpt, Gtk::PACK_SHRINK);
	//button_box->pack_end(*b_queue, Gtk::PACK_SHRINK);


	//b_queue->show();
	b_jumpt->show();
	b_close->show();
	button_box->show();

	sep->show();

	tree_view_.show();
	scroll_.show();

	entry_.show();

	main_box->show();
}

void X2JWindow::AddEntry(int position, strref name) {
	entries_.push_back(make_pair(position, name));
}

void X2JWindow::Display() {
	Reset();
	Gtk::Main::run(*this);
}

void X2JWindow::Reset() {
	tree_model_->clear();

	for ( vector< pair<int, string> >::iterator it = entries_.begin() ;
			it != entries_.end() ; ++it ) {
		Gtk::TreeModel::Row row = *(tree_model_->append());
		row[pos_column_] = it->first;
		row[name_column_] = it->second;
	}
}

bool X2JWindow::on_window_key_press(GdkEventKey* event) {
	if (event->keyval == GDK_Escape) {
		hide();
	}
	return true;
}

void X2JWindow::on_queue() {
	std::cout << "Queue" << std::endl;
}

void X2JWindow::on_jumpt() {
	Gtk::TreeModel::iterator row_it =
		tree_view_.get_selection()->get_selected();
	if (row_it) {
		int pos = row_it->get_value(pos_column_);
		hide();
		ItemSelected(pos);
	}
}

void X2JWindow::on_close() {
	hide();
}

void X2JWindow::on_change() {
	tree_view_.get_selection()->unselect_all();
	Gtk::Adjustment* vert = scroll_.get_vadjustment();
	vert->set_value(vert->get_lower());

	strref text = entry_.get_text().raw();
	const vector<int>& survivors = Search(entry_.get_text().raw());
	vector<int>::const_iterator surv_it = survivors.begin();

	if (!IsCharAppend(text)) Reset();
	Gtk::TreeModel::Children children = tree_model_->children();
	Gtk::TreeModel::Children::iterator row_it = children.begin();
	while ( row_it != children.end() ) {
		int pos = row_it->get_value(pos_column_);
		// Advance survivor iterator.
		while ( surv_it != survivors.end() && *surv_it < pos ) ++surv_it;
		if (surv_it == survivors.end()) {
			while ( row_it != children.end() ) {
				row_it = tree_model_->erase(row_it);
			}
			break;
		}
		if (*surv_it != pos) row_it = tree_model_->erase(row_it);
		else ++row_it;
	}
}

void X2JWindow::on_activate_entry() {
	Gtk::TreeModel::Children children = tree_model_->children();
	Gtk::TreeModel::Children::iterator row_it = children.begin();
	if (row_it != children.end()) {
		int pos = row_it->get_value(pos_column_);
		hide();
		ItemSelected(pos);
	}
}

void X2JWindow::on_activate_treeview(
		const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* /* column */) {

  Gtk::TreeModel::iterator row_it = tree_model_->get_iter(path);
  if (row_it) {
		int pos = row_it->get_value(pos_column_);
		hide();
		ItemSelected(pos);
  }
}


Frontend* Frontend::get() {
	int argc = 1;
	char *args[] = { "xmms2_jump", NULL };
	char **argv = &args[0];
	static Gtk::Main kit(argc, argv);
	return new X2JWindow();
}
