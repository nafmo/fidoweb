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
#include <string.h>
#include <stdlib.h>
#include <iostream.h>
#include "config.h"

#define BUFFER 64000

char *area, *msgid, *name, *subject, *body;

// Read CGI POST input
void readinput(void)
{
    int         ch;
    char        buf[BUFFER], hex[3] = "00", *data_p = NULL;
    unsigned    i = 0;

    area = NULL;
    msgid = NULL;
    name = NULL;
    subject = NULL;
    body = NULL;

    do
    {
        ch = getchar();
        switch (ch)
        {
            case '+': // Escaped space
                buf[i ++] = ' ';
                break;

            case '%': // Escaped hexcode
                hex[0] = getchar();
                hex[1] = getchar();
                sscanf(hex, "%xd", &ch);
                buf[i ++] = ch;
                break;

            case '&': // end of this input
            case EOF:
                if (i > 0) {
                    buf[i] = 0;
                    if (strcmp(buf, "area") == 0)
                    {
                        area = strdup(data_p);
                    }

                    if (strcmp(buf, "msgid") == 0)
                    {
                        msgid = new char[(strlen(data_p) >> 1) + 1];
                        if (msgid)
                        {
                            unsigned j;
                            for (j = 0; j < strlen(data_p); j += 2)
                            {
                                hex[0] = data_p[j];
                                hex[1] = data_p[j + 1];
                                sscanf(hex, "%xd", &ch);
                                msgid[j >> 1] = (char) ch;
                            }
                            msgid[j >> 1] = 0;
                        }
                    }

                    if (strcmp(buf, "name") == 0)
                    {
                        name = strdup(data_p);
                    }

                    if (strcmp(buf, "subject") == 0)
                    {
                        subject = strdup(data_p);
                    }

                    if (strcmp(buf, "body") == 0)
                    {
                        body = strdup(data_p);
                    }

                    // Start over
                    i = 0;
                }
                break;

            case '=': // start of data
                buf[i ++] = 0; // string separator
                data_p = &buf[i];
                break;

            default:
                buf[i ++] = ch;
                break;
        }

        if (i == BUFFER)
        {
            // Buffer overrun
            return;
        }
    } while (EOF != ch);
}
