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
#include <fstream.h>
#include "config.h"
#include "select.h"

// getpath
// - retrieves the path for an area by area tag
//   returns: pointer to static char * on success, NULL on failure
char *getpath(string name)
{
    char iname[256], tmp[256];
    static char ipath[256];

    // Open the area definitions file
    fstream input(AREADEF, ios::in);
    if (!(input.is_open()))
    {
        return NULL;
    }

    while (!input.eof())
    {
        input.getline(iname, 256);
        input.getline(ipath, 256);
        input.getline(tmp,   256);

        // If we have a match of area tags, return the path
        if (iname == name && iname != string("<hr>"))
        {
            input.close();
            return ipath;
        }
    }
    input.close();

    // We didn't find anything
    return NULL;
}
