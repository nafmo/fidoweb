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

#include <stdio.h>

#include "htmlerror.h"

struct
{
    const string title, message1, message2;
} errors[] =
{
    { "Bad request method",
      "The document was requested incorrectly. Please check the referring "
      "document for errors.",
      "" },
    { "Unkown area",
      "The area you tried to access, ",
      ", was not found. Please check the "
      "referring document for errors." },
    { "Cannot open area",
      "The area you tried to access, ",
      ", could not be read. "
      "It probably is being accessed by another process. "
      "Please try again. If the problem "
      "persists, please contact the SysOp." },
    { "Nonexisting UMSGID",
      "The message you tried to access (UMSGID ",
      ") does not exist. It might have expired. "
      "If you think this problem is issued in error, please "
      "contact the SysOp." },
    { "Nonexisting message",
      "The message you tried to access (# ",
      ") does not exist. "
      "It might have expired. "
      "If you think this problem is issued in error, please "
      "contact the SysOp." },
    { "Out of memory",
      "The system ran out of memory when trying to allocate memory for ",
      "" },
    { "No parameters given",
      "Required arguments are missing from the request. Please check "
      "the referring document for errors.",
      "" },
    { "Illegal parameter format",
      "The document was requested with illegal arguments: ",
      ". Please check the referring document for errors." },
    { "Required parameter missing",
      "The document was requested without the required parameter ",
      ". Please check the referring document for errors." },
    { "Not logged in",
      "You must be correctly logged in to access this function.",
      "" },

    { "Incorrect error message",
      "An error occured, and when the program tried to tell it, something "
      "else went wrong. This should not happen. If it did anyway, please "
      "contact the SysOp, stating exactly what you did.",
      "" }
};

void htmlerror(int error, const string data)
{
    cout << "Pragma: no-cache" << endl;
    cout << "Cache-Control: no-cache" << endl;
    cout << "Content-type: text/html" << endl;
    cout << "Refresh: 600" << endl;
    cout << endl;

    if (error < 0 || error >= MAXERROR) error = BADERROR;

    cout << "<html>" << endl;
    cout << "<head>" << endl;
    cout << " <title>" << errors[error].title << "</title>" << endl;
    cout << " <link rel=\"stylesheet\" href=\"fido.css\" type=\"text/css\">"
         << endl;
    cout << "</head>" << endl;
    cout << "<body>" << endl;
    cout << "<h1>An error has occured</h1>" << endl;
    cout << "<p>Your request could not be fulfilled." << endl;
    cout << "<p>" << errors[error].message1 << data
         << errors[error].message2 << endl;
    cout << "</html>" << endl;
    exit(0);
}

void htmlerrori(int error, const int data)
{
    char tmp[64];
    sprintf(tmp, "%d", data);
    htmlerror(error, tmp);
}
