#!/bin/sh

# xmms2_jump: Quickly search your XMMS2 playlist.
# Copyright (C) 2007  David Scott Reiss
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


NAMES_FILE="names.dat"
INDEX_FILE="index.dat"

if [ -z "$HOME" -a -z "$XDG_CACHE_HOME" ] ; then
	echo 1>&2 "You must define XDG_CACHE_HOME or at least HOME."
	exit 1
fi

MY_XDG_CACHE_HOME="${XDG_CACHE_HOME-${HOME}/.cache}"
CACHE_DIR="$MY_XDG_CACHE_HOME/xmms2_jump"

if [ -f "$CACHE_DIR/$NAMES_FILE" -a  -f "$CACHE_DIR/$INDEX_FILE" ] ; then
	xmms2_jump-query
else
	xmms2_jump-makedb && xmms2_jump-query
fi
