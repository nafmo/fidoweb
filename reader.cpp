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
#include <fstream.h>
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

void display(string area, int msgno, bool framed, bool pre, bool reply,
             bool showkludges);
void writeform(XMSG &msg, const unsigned int *trans, string &area,
               string &msgid, bool, bool);

// main
int main(int argc, char **)
{
    if (argc != 1)
    {
        htmlerror(BADREQUEST, "");
    }

    // Retrieve parameters
    // reader.exe?area=R20_INTRESSE&msg=10[&frame=1]
    string area = getarg("area");
    string msg  = getarg("msg");
    string frame= getarg("frame", true);
    string pre_s= getarg("pre", true);
    string quo_s= getarg("reply", true);
    string klu_s= getarg("kludges", true);

    bool framed = frame == "1";
    bool pre    = pre_s == "1";
    bool reply  = quo_s == "1";
    bool kludges= klu_s == "1";

    unsigned msgno;
    sscanf(msg.c_str(), "%d", &msgno);

    display(area, msgno, framed, pre, reply, kludges);
    return 0;
}

// display
// - prints the selected message
void display(string area, int msgno, bool framed, bool pre, bool reply,
             bool showkludges)
{
    char *p, *qp, *cp;

    // Lookup who's reading so we can highlight personal messages
    // and hide personal messages to others
    char *me_p = getenv("REMOTE_USER");
    string me = me_p;

    // Check if we have a guest user
    bool isguest = (strcmp(me_p, GUEST) == 0);
    if (isguest)
    {
        me_p = NULL;
        reply = false;
    }

    if (!me_p) me_p = "";

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
    word msgtype = MSGTYPE_SQUISH;

    areahandle = MsgOpenArea((unsigned char *) areapath, MSGAREA_NORMAL,
                             msgtype);

    if (!areahandle)
    {
        htmlerror(CANNOTOPENAREA, areapath);
    }

    MsgLock(areahandle);

    HMSG msghandle;
    XMSG msg;
    dword ctrllen, msglen;
    char *ctrlbuf, *msgbuf;

    // Convert the UMSGID to a local message number
    dword lmsgno = MsgUidToMsgn(areahandle, msgno, UID_EXACT);
    if (!lmsgno)
    {
        htmlerrori(NONEXISTINGUMSGID, msgno);
    }

    msghandle = MsgOpenMsg(areahandle, MOPEN_READ, lmsgno);

    if (!msghandle)
    {
        htmlerrori(NONEXISTINGMESSAGE, lmsgno);
    }

    // Check the length of the control information and allocate space for it
    ctrllen = MsgGetCtrlLen(msghandle);
    ctrlbuf = new char[ctrllen];
    if (!ctrlbuf)
    {
        htmlerror(ALLOCATEFAILED, "the control data.");
    }

    // Check the length of the message body and allocate space for it
    msglen = MsgGetTextLen(msghandle);
    msgbuf = new char[msglen];
    if (!msgbuf)
    {
        htmlerror(ALLOCATEFAILED, "the message body.");
    }

    // Read message headers, control data and body
    MsgReadMsg(msghandle, &msg, 0L, msglen, (unsigned char *) msgbuf,
               ctrllen, (unsigned char *) ctrlbuf);

    // Check character set
    const unsigned int *trans = chrscheck(ctrlbuf);

    // Override for Russian areas
    if (c_437 == trans && 0 == area.find("RU.")) trans = c_866;

    // Translate sender and recipient names
    string from = conv(msg.from, trans);
    string to   = conv(msg.to,   trans);

    // Check if its a private message to someone else
    bool allow = true;
    if (msg.attr & MSGPRIVATE && me != to)
    {
        allow = false;
    }

    // Retrieve high message number
    dword high = MsgHighMsg(areahandle);

    // Locate the previous and next unique message ids.
    UMSGID prev = MsgMsgnToUid(areahandle, lmsgno - 1),
           next = MsgMsgnToUid(areahandle, lmsgno + 1);

    // Print out HTML header
    cout << "Content-type: text/html" << endl;
    cout << endl;

    cout << "<html>" << endl;
    cout << "<head>" << endl;
    if (allow)
    {
        cout << " <title>"
             << (reply ? "Reply: " : "")
             << conv(msg.subj, trans) << " ("
             << conv(msg.from, trans) << ")</title>" << endl;
    }
    else
    {
        cout << " <title>Private message</title>" << endl;
    }
    cout << " <meta name=\"generator\" content=\"reader.exe\">" << endl;
    cout << " <link rel=\"stylesheet\" href=\"fido.css\" type=\"text/css\">"
         << endl;
    cout << "</head>" << endl;
    cout << "<body>" << endl;

    string msgid = "";

    if (reply && allow)
    {
        // When we're replying to a message, we need to locate the
        // MSGID, so it can be passed to the reply writer program for
        // proper reply linking.
        unsigned char *pp = (unsigned char *) strstr(ctrlbuf, "\x1""MSGID: ");
        if (pp && *pp)
        {
            pp += 8;
            while (*pp && *pp != 1)
            {
                msgid += "0123456789abcdef"[(*pp) >> 4];
                msgid += "0123456789abcdef"[(*pp) & 0x0f];
                pp ++;
            }
        }

        cout << "<h1>Reply in " << area << "</h1>" << endl;

        // Write the top of the input form
        writeform(msg, trans, area, msgid, framed, true);
    }
    else if (allow)
    {
        // When we're just reading a message, print the headers in a
        // nicely formatted table
        cout << "<table border=0 class=title>" << endl;

        // Link to the following message.
        if (next && lmsgno < high)
        {
            cout << " <tr><td align=center valign=top>"
                    "[ <a href=\"reader.exe?area="
                 << area << "&amp;msgno=" << next
                 << "&amp;kludges=" << (showkludges ? '1' : '0')
                 << (framed ? "&amp;frame=1\"" : "\"")
                 << ">Next</a> ]";
        }
        else // Or back to the area list
        {
            cout << " <tr><td align=center valign=top>"
                    "[ <a href=\"arealist.exe?frame="
                 << (framed ? '1' : '0') <<"\">Areas</a> ]" << endl;
        }

        cout << "<th align=right valign=top>Area:<td valign=top>" << area
             << "<th align=right valign=top>Number:<td valign=top>"
             << lmsgno << '/' << high;

        cout << endl;

        if (prev)
        {
            cout << " <tr><td align=center valign=top>"
                    "[ <a href=\"reader.exe?area="
                 << area << "&amp;msgno=" << prev
                 << "&amp;kludges=" << (showkludges ? '1' : '0')
                 << (framed ? "&amp;frame=1\"" : "\"")
                 << ">Previous</a> ]";
        }
        else
        {
            cout << " <tr><td> ";
        }

        cout << "<th align=right valign=top>From:<td valign=top>"
             << (me == from ? "<b>" : "")
             << from
             << (me == from ? "</b>" : "");

        cout.form(" <th align=right valign=top>Written:<td valign=top>"
                  "%04u-%02u-%02u %u:%02u:%02u",
                  msg.date_written.date.yr + 1980,
                  msg.date_written.date.mo,
                  msg.date_written.date.da,
                  msg.date_written.time.hh,
                  msg.date_written.time.mm,
                  msg.date_written.time.ss << 1);
        cout << endl;

        // Link back to the message list for the area.
        cout << " <tr><td align=center>[ <a href=\"messages.exe?area="
             << area << "&amp;start=" << lmsgno
             << (framed ? "&amp;frame=1\" target=\"upper\">"
                          "List</a> |"
                        : "\">List</a> ]") << endl;

        cout << "<th align=right valign=top>To:<td valign=top>"
             << (me == to   ? "<b>" : "")
             << to
             << (me == to   ? "</b>" : "");

        if (msg.date_arrived.date.mo)
            cout.form(" <th align=right>Arrived:<td>%04u-%02u-%02u %u:%02u:%02u",
                      msg.date_arrived.date.yr + 1980,
                      msg.date_arrived.date.mo,
                      msg.date_arrived.date.da,
                      msg.date_arrived.time.hh,
                      msg.date_arrived.time.mm,
                      msg.date_arrived.time.ss << 1);
        else
            cout << "<th><td>";

        cout << endl;

        cout << " <tr><th align=right valign=top>Subject:"
                "<td valign=top colspan=2>"
             << conv(msg.subj, trans)
             << " <th align=right valign=top>Attr:<td valign=top>";

        if (msg.attr & MSGPRIVATE)  cout << "prv ";
        if (msg.attr & MSGCRASH)    cout << "cra ";
        if (msg.attr & MSGSENT)     cout << "snt ";
        if (msg.attr & MSGFILE)     cout << "f/a ";
        if (msg.attr & MSGFWD)      cout << "tra ";
        if (msg.attr & MSGORPHAN)   cout << "orp ";
        if (msg.attr & MSGKILL)     cout << "k/s ";
        if (msg.attr & MSGLOCAL)    cout << "loc ";
        if (msg.attr & MSGHOLD)     cout << "hld ";
        if (msg.attr & MSGFRQ)      cout << "frq ";
        if (msg.attr & MSGURQ)      cout << "urq ";
        if (msg.attr & MSGSCANNED)  cout << "scn ";

        cout << (char) 160 << endl;

        cout << "</table>" << endl;
        cout << "<hr>" << endl;

        if (showkludges)
        {
            // Print control info ("kludge lines")
            cout << "<small>" << endl;
            p = ctrlbuf;
            if (1 == *p) p ++;
            while (*p)
            {
                if (1 == *p)
                {
                    cout << "<br>" << endl;
                    p ++;
                }
                else
                {
                    cout << *(p ++);
                }
            }
            cout << "</small>";
        }
    }
    else
    {
        // This is a private message
        cout << "You are not allowed to view this private message." << endl;
    }

    // Print body
    p = msgbuf;
    bool isquote = false, iskludge = false, isfirst = true;
    char *lastbreak = p;
    unsigned int c;

    if (allow && pre) cout << "<pre>";

    // Loop over the body buffer
    if (allow)
    {
        char *http = strstr(p, "http://"), *endhttp = NULL;

        while (*p)
        {
            if (13 == *p || isfirst)
            {
                // Handle line breaks (CRs).
                // We also wind up here on our very first call (to set up
                // some stuff).

                // Close any open link
                if (endhttp)
                {
                    cout << "</a>" << endl;
                    endhttp = NULL;
                }

                // Close the boldface attribute if we were reading a quoted
                // line.
                if (isquote && !reply)
                {
                    isquote = false;
                    if (!reply) cout << "</b>";
                }

                // Close the small (or italics) attribute if were reading a
                // control ("kludge") line
                if (iskludge && !reply && showkludges)
                {
                    if (pre)
                    {
                        cout << "</i>";
                    }
                    else
                    {
                        cout << "</small>";
                    }
                }

                // Terminate the line.
                if (pre)
                {
                    // Preformatted display just puts in a linebreak.
                    if (!iskludge || showkludges)
                        cout << endl;
                }
                else if (reply)
                {
                    // When in reply mode, we don't put out line breaks after
                    // kludges, since we aren't showing them.
                    if (!iskludge && !isfirst)
                        cout << endl;
                }
                else
                {
                    // If the line break comes right after the previous one,
                    // we need to fool the HTML parser to do a proper line
                    // break (several <br>s after another collapse into one).
                    if (!iskludge || showkludges)
                    {
                        if (lastbreak == p && !isfirst)
                            cout << ' ';

                        if (showkludges || !isfirst)
                            cout << "<br>" << endl;
                    }
                }

                // If we were at the first line, indicate that we aren't
                // any longer, and if we weren't, then skip over the CR
                if (!isfirst) p ++;
                else isfirst = false;

                // Check whether the current line is a quote line or not
                qp = strchr(p, '>');
                cp = strchr(p, 13);
                if (qp && (!cp || qp < cp) && qp - p < 6)
                {
                    if (!reply)
                    {
                        // Only boldface quotes when we aren't replying
                        cout << "<b class=quote>";
                    }
                    isquote = true;
                }
                else if (reply) // && lastbreak != p
                {
                    // We come here if we are replying (=quoting), and the
                    // previous line was not a quote.

                    if (iskludge)
                    {
                        // If last line was a kludge line, we didn't quote it,
                        // so just set the flag to indicate that we are done
                        // with it.
                        iskludge = false;
                    }
                    else
                    {
                        // Prepend quote initials to the current line.
                        cout << ' ' << from[0];
                        for (unsigned i = 1; i < from.length(); i ++)
                        {
                            if (' ' == from[i - 1])
                                cout << from[i];
                        }

                        cout << "&gt; ";
                    }
                }

                // We're done with the line, so we don't need to know
                // that it's a kludge anymore
                iskludge = false;

                // The SEEN-BY control line does not start with a 0x01
                // character, so it needs to be located especially.
                if (0 == strncmp(p, "SEEN-BY: ", 9))
                {
                    if (showkludges)
                    {
                        if (pre)
                        {
                            cout << "<i>";
                        }
                        else if (!reply)
                        {
                            cout << "<small>";
                        }
                    }
                    iskludge = true;
                }

                lastbreak = p;
            }
            else if (1 == *p)
            {
                // We just found ourselves a kludge line.
                if (!iskludge)
                {
                    if (showkludges)
                    {
                        if (pre)
                        {
                            cout << "<i>";
                        }
                        else if (!reply)
                        {
                            cout << "<small>";
                        }
                    }
                    iskludge = true;
                }
                else
                {
                    if (pre || reply)
                    {
                        cout << endl;
                    }
                    else
                    {
                        cout << "<br>" << endl;
                    }
                }
                p ++;
            }
            else
            {
                // Check if we have a HTTP address reference in the body
                if (http == p && !reply)
                {
                    cout << "<a href=\"";
                    char *pp;
                    for (pp = p;
                         *pp        && *pp != 13  && *pp != 10  &&
                         *pp != 32  && *pp != ')' && *pp != '>' &&
                         *pp != 34;
                         pp ++)
                        cout << *pp;
                    cout << "\">";

                    // Set http ending pointer and locate next link
                    endhttp = pp;
                    http = strstr(p + 1, "http://");
                }

                // This is normal body text.
                // Convert it to ISO 8859-1/Unicode for display.
                c = trans[(unsigned char) *(p ++)];

                if ((reply || !showkludges) && iskludge)
                {
                    // Do nothing if we are reading through a control line
                    // in reply mode or when not showing them.
                }
                else if (c == (unsigned int) '<')   // Special care for '<'
                    cout << "&lt;";
                else if (c == (unsigned int) '>')   // Special care for '>'
                {
                    cout << "&gt;";
                    if (isquote && reply)
                    {
                        // If we're replying, we add another '>' to the
                        // quoted material, to indicate that it's quoted
                        // for yet another generation.
                        cout << "&gt;";
                        isquote = false;
                    }
                }
                else if (c == (unsigned int) '&')   // Special care for '&'
                    cout << "&amp;";
                else if (c < 256)                   // Normal ISO-8859-1 character
                    cout << (char) c;
                else                                // Handle Unicode (ugly!)
                    cout << "&#" << c << ';';

                // Check if we want to close an http link
                if (endhttp == p)
                {
                    cout << "</a>";
                    endhttp = NULL;
                }
            }
        }

        // Close any left-over tags
        if (endhttp) cout << "</a>" << endl;
        if (iskludge && !reply) cout << "</b>";
        if (isquote && !reply && showkludges)
        {
            if (pre)
                cout << "</i>";
            else
                cout << "</small>";
        }
        if (pre) cout << "</pre>";

        cout << endl;
    }

    // Write the bottom half of the reply form if that is what we are
    // doing.
    if (reply  && allow)
    {
        writeform(msg, trans, area, msgid, framed, false);
    }

    // Some nice sugar on the bottom with links to other messages.
    cout << "<hr>" << endl;

    // The form for replying is essentially just a link to this
    // program, but it looks cool with a button :-)
    if (!reply && allow && !isguest)
    {
        cout << "<table border=0>" << endl;
        cout << " <tr><td valign=top>" << endl;

        cout << "  <form action=\"reader.exe\" method=\"get\">" << endl;
        cout << "   <input type=\"hidden\" name=\"area\" value=\""
             << area << "\">" << endl;
        cout << "   <input type=\"hidden\" name=\"msg\" value="
             << msgno << '>' << endl;
        cout << "   <input type=\"hidden\" name=\"kludges\" value="
             << (showkludges ? '1' : '0') << '>' << endl;
        cout << "   <input type=\"hidden\" name=\"frame\" value="
             << (framed ? '1' : '0') << '>' << endl;
        cout << "   <input type=\"hidden\" name=\"reply\" value=1>" << endl;
        cout << "   <input type=\"submit\" value=\"Write reply\">" << endl;
        cout << "  </form>" << endl;

        cout << " <td valign=top>" << endl;
    }

    // Close the message and area (we are done with it).
    delete msgbuf;
    delete ctrlbuf;
    MsgCloseMsg(msghandle);
    MsgCloseArea(areahandle);
    MsgCloseApi();

    // Provide for an option to display this message preformatted,
    // and to show kludges.
    if (allow)
    {
        cout << "  [ <a href=\"reader.exe?area=" << area
             << "&amp;msgno=" << msgno
             << "&amp;kludges=" << (showkludges ? '1' : '0')
             << (framed ? "&amp;frame=1" : "")
             << ((pre || reply) ? "\">Normal"
                                : "&amp;pre=1\">Preformatted")
             << "</a> |"
             << endl;

        cout << "  <a href=\"reader.exe?area=" << area
             << "&amp;msgno=" << msgno
             << "&amp;kludges=" << (showkludges ? '0' : '1')
             << (framed ? "&amp;frame=1\">" : "\">")
             << (showkludges ? "Hide" : "Show")
             << " kludges</a> |"
             << endl;
    }

    // Link to the previous message.
    if (prev)
    {
        cout << "  <a href=\"reader.exe?area=" << area << "&amp;msgno="
             << prev
             << "&amp;kludges=" << (showkludges ? '1' : '0')
             << (framed ? "&amp;frame=1\"" : "\"")
             << ">&lt;--Back</a> |" << endl;
    }

    // Link to the message to which this replies.
    if (msg.replyto)
    {
        cout << "  <a href=\"reader.exe?area=" << area
             << "&amp;msgno=" << msg.replyto
             << "&amp;kludges=" << (showkludges ? '1' : '0')
             << (framed ? "&amp;frame=1\"" : "\"")
             << ">Original</a> |"
             << endl;
    }

    // Link to replies to this message.
    if (msg.replies[1])
    {
        cout << "  Replies: ";

        for (int i = 0; i < 9 && msg.replies[i] != 0; i ++)
        {
            cout << "<a href=\"reader.exe?area=" << area
                 << "&amp;msgno=" << msg.replies[i]
                 << "&amp;kludges=" << (showkludges ? '1' : '0')
                 << (framed ? "&amp;frame=1\"" : "\"")
                 << ">" << i + 1
                 << "</a> " << endl;
        }
        cout << "| " << endl;
    }
    else if (msg.replies[0])
    {
        cout << "  <a href=\"reader.exe?area=" << area
             << "&amp;msgno=" << msg.replies[0]
             << "&amp;kludges=" << (showkludges ? '1' : '0')
             << (framed ? "&amp;frame=1\"" : "\"")
             << ">Reply</a> |"
             << endl;
    }

    // Link to the following message.
    if (next && lmsgno < high)
    {
        cout << "  <a href=\"reader.exe?area=" << area << "&amp;msgno="
             << next
             << "&amp;kludges=" << (showkludges ? '1' : '0')
             << (framed ? "&amp;frame=1\"" : "\"")
             << ">Next--&gt;</a> |" << endl;
    }
    else
    {
        cout << "  No more messages in this area |" << endl;
    }

    // Link to post a new message.
    cout << "  <a href=\"new.exe?area=" << area
         << (framed ? "&amp;frame=1\" target=\"_new\"" : "\"")
         << ">Write new message</a> |" << endl;

    // Link back to the message list for the area.
    cout << "  <a href=\"messages.exe?area=" << area << "&amp;start="
         << lmsgno
         << (framed ? "&amp;frame=1\" target=\"upper\">"
                      "Update message list</a> |"
                    : "\">Message list</a> |") << endl;

    // Link back to the area list.
    cout << "  <a href=\"arealist.exe?frame="
         << (framed ? '1' : '0') <<"\">Area list</a> ]" << endl;

    // Close the table (if any)
    if (!reply && allow && !isguest) cout << "</table>" << endl;

    // Close the HTML code
    cout << "</body>" << endl;
    cout << "</html>" << endl;

    // Set lastread pointer
    if (!reply && !isguest)
    {
        char *p = getenv("REMOTE_USER");
        setlastread(p, area, msgno);
    }
}

// writeform
// - write out the form in which we write replies
void writeform(XMSG &msg, const unsigned int *trans, string &area,
               string &msgid, bool framed, bool upper)
{
    if (upper)
    {
        // The upper half.
        string subject = conv(msg.subj, trans);
        if (subject == string("")) subject = string("(missing)");
        if (subject.substr(0, 4) == string("Re: "))
            subject = subject.substr(4);

        cout << "<form action=\"reply.exe\" method=\"post\" "
             << (framed ? "target=\"_new\"" : "")
             << ">" << endl;
        cout << " <input type=\"hidden\" name=\"msgid\" value=\"" << msgid
             << "\">" << endl;
        cout << " <input type=\"hidden\" name=\"name\" value=\""
             << conv(msg.from, trans) << "\">" << endl;
        cout << " Subject:<br>" << endl;
        cout << " <input type=\"text\" name=\"subject\" value=\""
             << subject << "\" maxlen=75 size=75>" << endl;
        cout << " <p>Message text:<br>" << endl;
        cout << " <textarea name=\"body\" cols=80 rows=25 wrap=\"virtual\" "
                "maxlength=32000>" << endl;
    }
    else
    {
        // The lower half.
        cout << " </textarea>" << endl;
        cout << " <br>Area:" << endl;

        cout << " <select name=\"area\">" << endl;

        // Selection of desstination area (for cross-replies).
        fstream input(AREADEF, ios::in);
        if (input.is_open())
        {
            char iname[256], tmp[256], ititle[256];

            while (!input.eof())
            {
                input.getline(iname, 256);
                if (input.eof()) break;
                input.getline(tmp,   256);
                input.getline(ititle,256);

                if (string("<hr>") != iname)
                {
                    cout << "  <option value=\"" << iname
                         << (iname == area ? "\" selected>" : "\">")
                         << ititle << endl;
                }
            }

            input.close();
        }
        else
        {
            cout << "  <option value=\"" << area << "\" selected>"
                 << area << endl;
        }
        cout << " </select>" << endl;

        cout << " <input type=\"submit\" value=\"Post reply\">" << endl;
        cout << " <input type=\"reset\"  value=\"Reset\">" << endl;

        cout << "</form>" << endl;
    }
}
