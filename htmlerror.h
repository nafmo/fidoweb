// Copyright (c) 1999 Peter Karlsson
//
// $Id$
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef __HTMLERROR_H
#define __HTMLERROR_H

#include <string>

#define BADREQUEST 0
#define UNKNOWNAREA 1
#define CANNOTOPENAREA 2
#define NONEXISTINGUMSGID 3
#define NONEXISTINGMESSAGE 4
#define ALLOCATEFAILED 5
#define NOPARAMETERS 6
#define ILLEGALFORMAT 7
#define MISSINGPARAM 8
#define NOTLOGGEDIN 9

#define MAXERROR 10
#define BADERROR MAXERROR

void htmlerror(int error, const string data);
void htmlerrori(int error, const int data);

#endif