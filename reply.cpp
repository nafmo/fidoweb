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

#include <stdlib.h>
#include <iostream.h>
#include <stdio.h>
#ifdef __EMX__
# include <io.h>
#else
# include <unistd.h>
#endif
#include <time.h>
#include <string.h>
#include <fstream.h>

#include "config.h"
#include "replycgi.h"
#include "pkthead.h"
#include "htmlerror.h"
#define ONLYTRANS
#include "convert.h"

#ifndef __EMX__
# define strnicmp strncasecmp
#endif

// Translation table ISO-8859-1 => Codepage 437
const unsigned int iso2dos[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
    38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
    74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
    92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107,
    108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
    122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135,
    136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 32, 173, 155, 156,
    36, 157, 124, 21, 34, 99, 166, 174, 170, 45, 114, 45, 248, 241, 253,
    51, 39, 230, 20, 249, 44, 49, 248, 175, 172, 171, 47, 168, 65, 65, 65,
    65, 142, 143, 146, 128, 69, 144, 69, 69, 73, 73, 73, 73, 68, 165, 79,
    79, 79, 79, 153, 120, 153, 85, 85, 85, 154, 89, 32, 225, 133, 160, 131,
    97, 132, 134, 145, 135, 138, 130, 136, 137, 141, 161, 140, 139, 32,
    164, 149, 162, 147, 111, 148, 246, 148, 151, 163, 150, 129, 121, 32,
    152
};

// Translation table ISO-8859-1 => ISO-8859-1 (yeah, yeah, I know it's stupid)
const unsigned int through[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
    38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
    74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
    92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107,
    108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
    122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135,
    136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163,
    164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177,
    178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205,
    206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219,
    220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
    234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247,
    248, 249, 250, 251, 252, 253, 254, 255
};

// main
int main(void)
{
    // Sanity check.
    if (!getenv("REQUEST_METHOD") ||
        0 != strcmp(getenv("REQUEST_METHOD"), "POST"))
    {
        htmlerror(BADREQUEST, "");
    }

    // Find out who is on the other side.
    char *p = getenv("REMOTE_USER");
    if (!p || strcmp(p, GUEST) == 0)
    {
        htmlerror(NOTLOGGEDIN, "");
    }

    // Collect the user input.
    readinput();

    // Check what time it is, the time is used both for the MSGID and
    // the PKT file name.
    time_t t = time(NULL);
    char path[256];
    sprintf(path, "%s%08x.pkt", INBOUND, (unsigned) t);
#ifdef __EMX__
    while (0 == access(path, 0))
#else
    while (0 == access(path, F_OK))
#endif
    {
        // If a PKT file with this name was found, try another name.
        // NOTE: This code can suffer a race condition!
        t ++;
        sprintf(path, "%s%08x.pkt", INBOUND, (unsigned) t);
    }

    // Open the PKT file for writing.
    FILE *f = fopen(path, "wb");

    // Print out some fancy HTML code.
    cout << "Content-type: text/html" << endl;
    cout << endl;

    cout << "<html>" << endl;
    cout << "<head>" << endl;
    cout << " <title>New message in " << area << "</title>" << endl;
    cout << " <meta name=\"generator\" content=\"reply.exe\">" << endl;
    cout << " <link rel=\"stylesheet\" href=\"fido.css\" type=\"text/css\">"
         << endl;
    cout << "</head>" << endl;
    cout << "<body>" << endl;
    cout << "<h1>New message in " << area << "</h1>" << endl;

    // Prepare the PKT file structures.
    pktheader_t hdr;
    pktmsg_t msg;

    memset(&hdr, 0, sizeof(hdr));
    memset(&msg, 0, sizeof(msg));

    // Prepare the packet header.
    hdr.orgnode = NODE;
    hdr.dstnode = DNODE;
    hdr.orgnet  = NET;
    hdr.dstnet  = DNET;
    hdr.qorgzone= hdr.orgzone = ZONE;
    hdr.qdstzone= hdr.dstzone = DZONE;
    hdr.orgpoint= POINT;
    hdr.dstpoint= DPOINT;
    strcpy((char *) hdr.password, PASSWD);

    struct tm *lt = localtime(&t);

    hdr.year  = lt->tm_year + 1900;
    hdr.month = lt->tm_mon;
    hdr.day   = lt->tm_mday;
    hdr.hour  = lt->tm_hour;
    hdr.min   = lt->tm_min;
    hdr.sec   = lt->tm_sec;

    hdr.prodcodl = NOPRODCODE;

    hdr.capword  = CAPWORD;
    hdr.capvalid = CAPVALID;

    hdr.pktver = PKTVER;

    // Write the packet header.
    fwrite(&hdr, sizeof(hdr), 1, f);

    // Prepare the mesage header.
    msg.pktver = PKTVER;
    msg.orgnode= NODE;
    msg.dstnode= DNODE;
    msg.orgnet = NET;
    msg.dstnet = DNET;

    strftime((char *) msg.datetime, sizeof(msg.datetime), "%d %b %y  %T", lt);

    // Write the message header.
    fwrite(&msg, sizeof(msg), 1, f);

    // Select character set.
    // FIXME: This should be configurable.
    const unsigned int *trans = through;
    if (area && strnicmp(area, "R20_", 4) == 0) trans = iso2dos;

    // Print something usable.
    cout << "<ul>" << endl;
    cout << " <li><b>Area:</b> " << area << endl;
    cout << " <li><b>From:</b> " << p << endl;
    cout << " <li><b>To:</b> " << name << endl;
    cout << " <li><b>Subject:</b> " << subject << endl;
    cout << "</ul>" << endl;
    cout << "<hr>" << endl;

    // Convert message information to the selected character set.
    string from = conv((unsigned char *) p,    trans);
    if (!name) name = "All";
    string to   = conv((unsigned char *) name, trans);
    string subj = conv((unsigned char *) subject, trans);

    // Locate what browser we use
    string brow = conv((unsigned char *) getenv("HTTP_USER_AGENT"), trans);
    string tear = brow;

    // Check for some special cases (Mozilla impostors)
    bool impostor = false;
    int find1 = brow.find("Opera"), find2 = brow.find("Mozilla");
    if (find1 != -1 && find2 != -1)
    {
        int find3 = brow.find(';', find1);
        if (find3 != -1)
            tear = brow.substr(find1, find3 - find1);
        else
            tear = brow.substr(find1, brow.length() - find1 + 1);
        impostor = true;
    }

    find1 = brow.find("MSIE");
    if (find1 != -1 && find2 != -1)
    {
        find2 = brow.find(';', find1);
        tear = brow.substr(find1, find2 - find1) + " [";
        impostor = true;
    }

    // Change the name to a more "Fidonet-style" name (no slash between
    // name and version number), and remove platform information.
    int slash = tear.find('/'), space = tear.find(' ');
    if (slash != -1 && (slash < space || -1 == space))
        tear[slash] = ' ';

    int bracket = tear.find('['), paren = tear.find('(');
    if (space != -1 &&
        (-1 == bracket || space < bracket) &&
        (-1 == paren   || space < paren) &&
        !impostor)
        tear = tear.substr(0, space);
    else if (bracket != - 1)
        tear = tear.substr(0, bracket);
    else if (paren != - 1)
        tear = tear.substr(0, paren);

    // Write the name of the recipient, sender and the subject.
    fwrite(to.c_str(),   to.length() + 1,   1, f);
    fwrite(from.c_str(), from.length() + 1, 1, f);
    fwrite(subj.c_str(), subj.length() + 1, 1, f);

    // Set up kludges.
    char kludges[256];
    sprintf(kludges, "AREA:%s\x0d"
            "\1""MSGID: %u:%u/%u.%u %08x\x0d"
            "%s%s%s"
            "\1""CHRS: %s\x0d"
            "\1""RFC-User-Agent: %s\x0d",
            area,
            ZONE, NET, NODE, POINT, (unsigned) t,
            msgid ? "\1""REPLY: " : "",
            msgid ? msgid : "",
            msgid ? "\x0d" : "",
            (trans == through) ? "LATIN-1 2" : "CP437 2",
            brow.c_str());
    fwrite(kludges, strlen(kludges), 1, f);

    // Write the message body to the packet (and to the document).
    unsigned char *q = (unsigned char *) body, c, oc;
    while (*q)
    {
        c = trans[oc = *(q ++)];
        if (13 == c)
        {
            // Let CRs go through, and filter out any following LFs.
            fputc(13, f);
            if (10 == *q) q ++;
            cout << "<br>" << endl;
        }
        else if (10 == c)
        {
            // If we get LFs only, save them as CRs.
            fputc(13, f);
            cout << "<br>" << endl;
        }
        else
        {
            fputc(c, f);
            if ('<' == oc)      cout << "&lt;";
            else if ('>' == oc) cout << "&gt;";
            else if ('&' == oc) cout << "&amp;";
            else                cout << oc;
        }
    }

    // The body has been handled.
    cout << "<hr>" << endl;

    // Read Origin file
    fstream originf(ORIGIN, ios::in);
    char originp[256];
    if (originf.is_open())
    {
        originf.getline(originp, 256);
        originf.close();
        if (originp[strlen(originp) - 1] == '\n')
            originp[strlen(originp) - 1] = 0;
        if (originp[strlen(originp) - 1] == '\r')
            originp[strlen(originp) - 1] = 0;
    }
    else
        strcpy(originp, "Fidoweb server");

    string origin = conv((unsigned char *) originp, trans);

    // Finish up the message.
    fwrite("\x0d""--- ", 5, 1, f);
    fwrite(tear.c_str(), tear.length(), 1, f);

    // Write the message tail.
    char tail[256];
    sprintf(tail, "\x0d"" * Origin: %s (%u:%u/%u.%u)\x0d"
                  "SEEN-BY: %u/%u\x0d\x01"
                  "PATH: %u/%u\x0d",
            origin.c_str(), ZONE, NET, NODE, POINT, NET, NODE, NET, NODE);

    fwrite(tail, strlen(tail), 1, f);

    fwrite("\0\0\0", 3, 1, f);
    fclose(f);

    // Finish up the HTML document.
    cout << "<p>Your message has been generated,"
            " but will not appear immediately in the area." << endl;

    char *r = getenv("HTTP_REFERER");
    if (r)
    {
        cout.form("<p><a href=\"%s\" onClick=\"history.back()\">Back</a>\n", r);
    }

    cout << "</body>" << endl;
    cout << "</html>" << endl;

    // Deallocate;
    delete msgid;
}
