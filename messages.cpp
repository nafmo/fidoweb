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

#include <iostream.h>
#include <time.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
extern "C" {
#include <msgapi.h>
}

#include "config.h"
#include "charset.h"
#include "convert.h"
#include "select.h"
#include "getarg.h"
#include "lastread.h"
#include "htmlerror.h"

void display(string area, unsigned msgno, bool framed);

// main
int main(int argc, char **)
{
    if (argc != 1)
    {
        htmlerror(BADREQUEST, "");
    }

    // Retrieve parameters
    // messages.exe?area=R20_INTRESSE&start=501[&frame=1]
    string area = getarg("area");
    string msg  = getarg("start", true);
    string frame= getarg("frame", true);

    bool framed = frame == "1";

    unsigned msgno;
    if (msg == "")
        msgno = 0;
    else
        sscanf(msg.c_str(), "%u", &msgno);

    display(area, msgno, framed);
    return 0;
}

// display
// - lists the messages in the area
void display(string area, unsigned msgno, bool framed)
{
    // Check if we have a guest user
    char *p = getenv("REMOTE_USER");
    bool isguest = !p || (strcmp(p, GUEST) == 0);

    // Lookup area
    char *areapath = getpath(area);
    if (!areapath)
    {
        htmlerror(UNKNOWNAREA, area);
    }

    // Open the MSGAPI
    struct _minf mi;
    memset(&mi, 0, sizeof(mi));
    //mi.def_zone = 2;
    MsgOpenApi(&mi);

    // Open the area
    HAREA areahandle;
    word msgtype = MSGTYPE;

    areahandle = MsgOpenArea((unsigned char *) areapath, MSGAREA_NORMAL,
                             msgtype);

    if (!areahandle)
    {
        htmlerror(CANNOTOPENAREA, areapath);
    }

    MsgLock(areahandle);

    // Read messages
    HMSG msghandle;
    XMSG msg;
    dword msgn, ctrllen, high = MsgHighMsg(areahandle);
    UMSGID uid;
    char *ctrlbuf;

    // Print the page header
    cout << "Content-type: text/html" << endl;
    cout << endl;
    cout << "<html>" << endl;
    cout << "<head>" << endl;
    cout << " <title>Messages in " << area << "</title>" << endl;
    cout << " <meta name=\"generator\" content=\"messages.exe\">" << endl;
    cout << " <link rel=\"stylesheet\" href=\"fido.css\" type=\"text/css\">"
         << endl;
    cout << "</head>" << endl;
    cout << "<body>" << endl;

    cout << "<h1>" << area << "</h1>" << endl;

    cout << high << " messages in area." << endl;

    // If no starting number was supplied, retrieve lastread and convert
    // it to a local message number
    if (0 == msgno && !isguest)
    {
        // Check who we are
        char *p = getenv("REMOTE_USER");
        if (p)
        {
            unsigned lastread = getlastread(p, area);

            if (lastread)
            {
                // Convert the UMSGID to a local message number
                dword lmsgno = MsgUidToMsgn(areahandle, lastread, UID_EXACT);
                if (lmsgno)
                {
                    // Valid number
                    msgno = lmsgno;
                }
            }
        }
    }

    // If the supplied starting number is 0, we start at the highest possible
    // multiple of 500 (+1) of messages.
    if (0 == msgno)
    {
        if (high > 500)
        {
            msgno = ((high - 1) / 500) * 500 + 1;
        }
        else
        {
            msgno = 1;
        }
    }

    // If there are more than 500 messages in the area, we only show
    // 500 of them, but provide links to lists of the other messages in
    // the area.
    if (high > 500)
    {
        // Tell which ones are displayed.
        cout << " Displaying messages " << msgno << '-'
             << ((msgno + 499 > high) ? high : msgno + 499)
             << '.' << endl;

        // Links to the rest.
        cout << "<br>[ Messages ";
        for (unsigned int i = 1; i <= high; i += 500)
        {
            cout << "<a href=\"messages.exe?area=" << area
                 << "&amp;start=" << i
                 << (framed ? "&amp;frame=1\"" : "\"")
                 << '>' << i << '-'
                 << ((i + 499 > high) ? high : i + 499)
                 << "</a>" << endl;
        }
        cout << ']' << endl;
    }

    // Print the table header
    cout << "<p>" << endl;
    cout << "<table>" << endl;
    cout << " <tr><th>#<th>From<th>To<th>Subject<th>Written" << endl;

    // Initialize counters and flags.
    int disp = 0;
    bool stopped = false;

    string me = p, from, to;

    // Loop over the available messages.
    for (msgn = msgno; msgn <= high; msgn ++)
    {
        // Open up the message for reading.
        msghandle = MsgOpenMsg(areahandle, MOPEN_READ, msgn);
        if (msghandle)
        {
            // Opening message succeeded.
            // Allocate memory for control information.
            ctrllen = MsgGetCtrlLen(msghandle);
            ctrlbuf = new char[ctrllen];

            // Read header and control information, but not body.
            MsgReadMsg(msghandle, &msg, 0L, 0, NULL,
                       ctrllen, (unsigned char *) ctrlbuf);

            // We need the unique message id to access the messages
            uid = MsgMsgnToUid(areahandle, msgn);

            // Check which character set is used and set up a translation
            // table.
            const unsigned int *trans = ctrlbuf ? chrscheck(ctrlbuf) : c_437;

            // Translate sender and recipient names
            from = conv(msg.from, trans);
            to   = conv(msg.to,   trans);

            // Check if its a private message to someone else
            bool allow = true;
            if (msg.attr & MSGPRIVATE && me != to)
            {
                allow = false;
            }

            // Print information about the message.
            cout << " <tr><td align=right valign=top>"
                    "<a href=\"reader.exe?area="
                 << area << "&amp;msgno=" << uid
                 << "&amp;kludges=0"
                 << (framed ? "&amp;frame=1\" target=\"lower\"" : "\"")
                 << '>' << msgn
                 << "</a>" << endl;

            if (allow)
            {
                cout << "  <td valign=top>"
                     << (me == from ? "<b>" : "")
                     << from
                     << (me == from ? "</b>" : "")
                     << endl;
            }
            else
            {
                cout << "  <td valign=top align=right>"
                        "Private message to:"
                     << endl;
            }
            cout << "  <td valign=top>"
                 << (me == to   ? "<b>" : "")
                 << to
                 << (me == to   ? "</b>" : "")
                 << endl;

            if (allow)
            {
                cout << "  <td valign=top>" << conv(msg.subj, trans) << endl;
                cout.form("  <td valign=top>%04u-%02u-%02u",
                          msg.date_written.date.yr + 1980,
                          msg.date_written.date.mo,
                          msg.date_written.date.da);
                cout << endl;
            }
            else
            {
                cout << "  <td><td>" << endl;
            }

            if (ctrlbuf) delete ctrlbuf;
            MsgCloseMsg(msghandle);

            disp ++;
        }

        // Never display more than 500 messages at a time.
        if (500 == disp)
        {
            stopped = true;
            break;
        }
    }

    // Close the MSGAPI.
    MsgCloseArea(areahandle);
    MsgCloseApi();

    // Close the table.
    cout << "</table>" << endl;
    cout << "<hr>" << endl;

    // If we stopped the display and have more to show, provide a link
    // to the continued list.
    if (stopped && msgn < high)
    {
        cout << "<a href=\"messages.exe?area=" << area << "&amp;start="
             << msgn + 1
             << (framed ? "&amp;frame=1\"" : "\"")
             << ">More messages</a> |" << endl;
    }

    // Link to post a new message.
    if (!isguest)
    {
        cout << "<a href=\"new.exe?area=" << area
             << (framed ? "&amp;frame=1\" target=\"_new\"" : "\"")
             << ">Write new message</a> |" << endl;
    }

    // Link to area list.
    if (framed)
        cout << "<a href=\"arealist.exe?frame=1\" "
                "target=\"lower\">Area list</a>"
             << endl;
    else
        cout << "<a href=\"arealist.exe\">Back to area list</a>" << endl;

    // Close the HTML code.
    cout << "</body>" << endl;
    cout << "</html>" << endl;
}
