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

// pkthead.h
// Header file that defines the layout of a standard type-2+ Fidonet PKT file.

#ifndef __PKTHEAD_H
#define __PKTHEAD_H
#include "datatyp.h"

// PKT revision
#define PKTVER 2

// Capability word, and capability word validity (byteflipped).
#define CAPWORD 0x0001
#define CAPVALID (((CAPWORD & 0xFF) << 8) | ((CAPWORD & 0xFF00) >> 8))

// Product code for "no product code allocated".
#define NOPRODCODE 0xFE

// Structure of the PKT file header.

#ifdef __EMX__
#pragma pack(1)
#endif

typedef struct Pktheader
{
    UINT16  orgnode, dstnode;        // Originating/destination node number.
    SINT16  year, month, day,        // Creation year, month (0-11), day.
            hour, min, sec,          // Creation hour, minute, second.
            baud, pktver;            // Transfer speed and PKT revision.
    UINT16  orgnet, dstnet;          // Originating/destination net number.
    UINT8   prodcodl, pvmajor,       // Product code (low), Major revision no.
            password[8];             // Password.
    UINT16  qorgzone, qdstzone;      // Originating/destination zone number.
    UINT8   filler[2];               // Filler (unused).
    UINT16  capvalid;                // Capability word validity check.
    UINT8   prodcodh, pvminor;       // Product code (high), Minor rev. no.
    UINT16  capword,                 // Capability word as of type 2+.
            orgzone, dstzone,        // Originating/destination zone number.
            orgpoint, dstpoint;      // Originating/destination point number.
    UINT8   proddata[4];             // Implementation specifict data.
} pktheader_t;

// Structure of the PKT message header (the fixed length fields).
typedef struct Pktmsg
{
    UINT16  pktver,                  // PKT revision (flag).
            orgnode, dstnode,        // Originating/destination zone number.
            orgnet, dstnet,          // Originating/destination net number.
            attribute, cost;         // Message attributes and cost.
    UINT8   datetime[20];            // Message date and time in string format.
} pktmsg_t;

#ifdef __EMX__
#pragma pack()
#endif

#endif
