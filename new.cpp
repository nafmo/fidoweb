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
#include <iostream.h>
#include <stdlib.h>
#include "config.h"
#include "getarg.h"

void display(string area);

// main
int main(void)
{
    // Retrieve parameters
    // new.exe?area=R20_INTRESSE
    string area = getarg("area");

    // Print the form
    display(area);

    return 1;
}

// display
// - print the message entry form
void display(string area)
{
    // Check if we have a guest user
    char *p = getenv("REMOTE_USER");
    bool isguest = (strcmp(p, GUEST) == 0);

    // HTTP preamble
    cout << "Content-type: text/html" << endl;
    cout << endl;

    // HTML document
    cout << "<html>" << endl;
    cout << "<head>" << endl;
    cout << " <title>Post to " << area << "</title>" << endl;
    cout << " <meta name=\"generator\" content=\"new.exe\">" << endl;
    cout << " <link rel=\"stylesheet\" href=\"fido.css\" type=\"text/css\">"
         << endl;
    cout << "</head>" << endl;
    cout << "<body>" << endl;

    cout << "<h1>Post to " << area << "</h1>" << endl;

    if (isguest)
    {
        cout << "You need an account to be allowed to post." << endl;
        cout << "</body>" << endl;
        cout << "</html>" << endl;
        return;
    }

    cout << " <form action=\"reply.exe\" method=\"post\">" << endl;
    cout << "  <input type=\"hidden\" name=\"area\" value=\"" << area
         << "\">" << endl;
    cout << "  <input type=\"hidden\" name=\"name\" value=\"All\">" << endl;
    cout << "  Subject:<br>" << endl;
    cout << "  <input type=\"text\" name=\"subject\" value=\"\" "
            "maxlen=75 size=75>" << endl;
    cout << "  <p>Message text:<br>" << endl;
    cout << "  <textarea name=\"body\" cols=80 rows=25 wrap=\"virtual\" "
            "maxlength=32000></textarea>" << endl;
    cout << "  <br>" << endl;
    cout << "  <input type=\"submit\" value=\"Post message\">" << endl;
    cout << "  <input type=\"reset\"  value=\"Reset\">" << endl;
    cout << " </form>" << endl;
    cout << "</body>" << endl;
    cout << "</html>" << endl;
}
