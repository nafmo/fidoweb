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

#include <fstream.h>
#include <stdio.h>
#include <string>
#include <io.h>

#include "config.h"
#include "lastread.h"

unsigned getlastread(string name, string echo)
{
    string fname = LASTREAD + name;

    fstream input(fname.c_str(), ios::in);
    if (!(input.is_open())) return 0;

    char iname[256], iread[256];

    while (!input.eof())
    {
        // File format:
        //  Line 1: Area tag
        //  Line 2: Lastread
        input.getline(iname, 256);
        if (input.eof()) break;
        input.getline(iread, 256);

        if (echo == iname)
        {
            unsigned retval;

            input.close();
            sscanf(iread, "%d", &retval);
            return retval;
        }
    }

    input.close();
    return 0;
}

void setlastread(string name, string echo, unsigned msgno)
{
    string fname = LASTREAD + name;
    string fnamebak = fname + ".bak";

    bool found = false;

    fstream input, output;

    if (access(fname.c_str(), 0))
    {
        output.open(fname.c_str(), ios::out);
        if (!(output.is_open())) return;
    }
    else
    {

        if (-1 == rename(fname.c_str(), fnamebak.c_str()))
        {
            remove(fnamebak.c_str());
            return;
        }

        input.open(fnamebak.c_str(), ios::in);
        if (!(input.is_open()))
        {
            rename(fnamebak.c_str(), fname.c_str());
            return;
        }

        output.open(fname.c_str(), ios::out);
        if (!(output.is_open()))
        {
            input.close();
            rename(fnamebak.c_str(), fname.c_str());
            return;
        }

        char iname[256], iread[256];

        while (!input.eof())
        {
            // File format:
            //  Line 1: Area tag
            //  Line 2: Lastread
            input.getline(iname, 256);
            if (input.eof()) break;
            output << iname << endl;
            input.getline(iread, 256);

            if (echo == iname)
            {
                output << msgno << endl;
                found = true;
            }
            else
                output << iread << endl;
        }
    }

    if (!found)
    {
        output << echo << endl;
        output << msgno << endl;
    }

    if (input.is_open()) input.close();
    output.close();

    remove(fnamebak.c_str());
}

