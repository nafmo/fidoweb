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
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <stdio.h>
#include "config.h"
#include "htmlerror.h"

// getarg
// - retrieves arguments from the QUERY_STRING
//   input: s: name of variable to find
//          optional: tells if the parameter is optional
//   returns: string with the value of the variable on success
//            doesn't return on failure
string getarg(char *s, bool optional)
{
    // Check that we actually *do* have any parameters
    char *request = getenv("QUERY_STRING");
    if (!request)
    {
        if (optional) return string("");

        htmlerror(NOPARAMETERS, "");
    }

    // Locate the variable in the request
    char *p = strstr(request, s);
    while (p)
    {
        // Variable name comes first or after an '&'
        if (p == request || '&' == *(p - 1))
        {
            // ok
            string str = "";
            // Locate '=' sign
            while (*p && *p != '=') p ++;
            // If we reached the end-of-string, something is wrong
            if (!*p)
            {
                if (optional) return string("");

                htmlerror(ILLEGALFORMAT, s);
            }
            p ++;

            // Retrieve the parameter
            while (*p && *p != '&')
            {
                if ('%' == *p)
                {
                    // Escaped hex code
                    static char hex[] = "00";
                    hex[0] = *(++ p);
                    hex[1] = *(++ p);
                    p ++;
                    unsigned c;
                    sscanf(hex, "%x", &c);
                    str += (char) c;
                }
                else
                {
                    // Normal
                    str += *(p ++);
                }
            }

            return str;
        }

        // find next
        p = strstr(p + 1, s);
    }

    // If we get here, we didn't find the parameter.
    // If it's an optional parameter, we just return an empty string
    if (optional) return string("");

    // Otherwise we die.
    htmlerror(MISSINGPARAM, s);
    return 0;
}

