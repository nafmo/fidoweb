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

#ifndef __CHARSET_H
#define __CHARSET_H

extern const unsigned int c_437[];
extern const unsigned int c_737[];
extern const unsigned int c_850[];
extern const unsigned int c_866[];
extern const unsigned int c_646_se[];
extern const unsigned int c_8859_1[];
extern const unsigned int c_atarist[];
extern const unsigned int c_macroman[];

const unsigned int *chrscheck(char *ctrlbuf);

#endif
