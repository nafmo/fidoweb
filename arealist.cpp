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
#include <stdlib.h>
#include "config.h"
#include "select.h"
#include "getarg.h"

// main
// - lists defined areas
int main(void)
{
    // Check if we have a guest user
    char *p = getenv("REMOTE_USER");
    bool isguest = p ? (strcmp(p, GUEST) == 0) : true;

    // HTTP preamble
    cout << "Content-type: text/html" << endl;
    cout << endl;

    // HTML body
    cout << "<html>" << endl;
    cout << "<head>" << endl;
    cout << " <title>List of areas</title>" << endl;
    cout << " <meta name=\"generator\" content=\"arealist.exe\">" << endl;
    cout << " <link rel=\"stylesheet\" href=\"fido.css\" type=\"text/css\">"
         << endl;
    cout << "</head>" << endl;
    cout << "<body>" << endl;

    // Retrieve parameters
    // arealist.exe[?frame=1]
    string frame = getarg("frame", true);
    bool framed = frame == "1";

    char iname[256], ipath[256], ititle[256];

    // Open area definition file
    fstream input(AREADEF, ios::in);
    if (!(input.is_open()))
    {
        cout << "Unable to read area file, please try again." << endl;
        cout << "</body></html>" << endl;
        return 0;
    }

    // Create a table containing all the areas
    cout << " <table>" << endl;
    cout << "  <tr><th>Title<th>Tag" << (isguest ? "" : "<th>Post")
         << endl;

    while (!input.eof())
    {
        // File format:
        //  Line 1: Area tag
        //  Line 2: Area path
        //  Line 3: Area title
        input.getline(iname, 256);
        if (input.eof()) break;
        input.getline(ipath, 256);
        if (input.eof()) break;
        input.getline(ititle, 256);

        // Area tag "<hr>" creates a horizontal ruler
        if (string("<hr>") == iname)
        {
            cout << "   <tr><td colspan=" << (isguest ? '2' : '3')
                 << "><hr>" << endl;
        }
        else
        {
            cout << "   <tr><td valign=top><a href=\"messages.exe?area="
                 << iname /*<< "&amp;start=0"*/
                 << (framed ? "&amp;frame=1\" target=\"upper\"" : "\"")
                 << '>' << ititle << "</a><br>" << endl;
            cout << "    <td valign=top>" << iname << "" << endl;
            if (!isguest)
            {
                cout << "    <td valign=top>[<a href=\"new.exe?area="
                     << iname << "\""
                     << (framed ? " target=\"_new\"" : "")
                     << ">Post</a>]" << endl;
            }
        }
    }
    input.close();

    // Close off the HTML code
    cout << " </table>" << endl;
    cout << "</body>" << endl;
    cout << "</html>" << endl;

    return 0;
}
