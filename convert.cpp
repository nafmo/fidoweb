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

#include <string>
#include <stdio.h>
#include "config.h"

// conv
// - converts a string
//   input: s - string to translate (char pointer)
//          tbl - conversion table
//   output: the converted string
string conv(unsigned char *s, const unsigned int *tbl)
{
    string r;
    unsigned int c;
    char buf[10];

    // Sanity check
    if (!s || !tbl) return r;

    // Loop over the string
    while (*s)
    {
        // Convert a character
        c = tbl[*(s ++)];

        // Take care of HTML specific codes
        if (c == (unsigned int) '<')
            r += "&lt;";
        else if (c == (unsigned int) '>')
            r += "&gt;";
        else if (c == (unsigned int) '&')
            r += "&amp;";
        else if (c < 256)
            r += (char) c;
        else
        {
            // Unicode
            sprintf(buf, "&#%u;", c);
            r += buf;
        }
    }
    return r;
}
