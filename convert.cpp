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
#include "charset.h"

#ifndef ONLYTRANS
// Check character set
// input: ctrlbuf - pointer to message control information
// output: pointer to translation table
const unsigned int *chrscheck(char *ctrlbuf)
{
    char *p;

    // Codepage 437 is fallback
    const unsigned int *trans = c_437;

    // Locate CHRS: control information
    p = strstr(ctrlbuf, "\x1""CHRS: ");
    if (p && *p)
    {
        p += 7;

        // Check for known character sets.
        if (0 == strncmp(p, "IBMPC", 5) ||
            0 == strncmp(p, "CP437", 5))
        {
            // "CHRS: IBMPC" may have an additional "CODEPAGE:"
            // specifier (FD*).
            p = strstr(ctrlbuf, "\x1""CODEPAGE: ");
            if (p)
            {
                p += 11;
                if (0 == strncmp(p, "850", 3)) trans = c_850;
                else trans = c_437;
            }
            else
                trans = c_437;
        }
        else if (0 == strncmp(p, "+7_FIDO", 7) ||
            0 == strncmp(p, "CP866", 5))
        {
            trans = c_866;
        }
        else if (0 == strncmp(p, "CP737", 5))
        {
            trans = c_737;
        }
        else if (0 == strncmp(p, "CP850", 5))
        {
            trans = c_850;
        }
        else if (0 == strncmp(p, "SWEDISH", 7))
        {
            trans = c_646_se;
        }
        else if (0 == strncmp(p, "LATIN-1", 7))
        {
            trans = c_8859_1;
        }
        else if (0 == strncmp(p, "MAC", 3))
        {
            trans = c_macroman;
        }
        else if (0 == strncmp(p, "ATARI", 5))
        {
            trans = c_atarist;
        }
    }

    return trans;
}
#endif

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
