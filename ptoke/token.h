/*  ptoke - Linux pbasic tokenizer,
    Copyright (C) 2003, Brett M. Gordon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define PTOKE_VERSION "0.35a"

#include "tokenizer.h"

struct BsoHeader{

  char Signature[4];
  byte Module;
  byte RecordCount;
};

struct BsoRecord{

  byte PacketCount;
  byte EEPROM[2048];
  byte EEPROMFlags[2048];
  byte VarCounts[4];
  byte PacketBuffer[2304];
  byte SlotNumber;
};

struct ProgramRecord{ 

  char *FileName;
  char FileType;  /* 0=BSO, 1=BSE */
  char *SourceBuffer;
  char *SourcePermBuffer;
  struct TModuleRec Record;
  byte SlotNumber;
  struct ProgramRecord *NextRecord;
};

struct fileattrib{
  byte ModuleType;
  byte SlotNumber;
};

#define GPL "Copyright (C) Brett M. Gordon \
\
This program is free software; you can redistribute it and/or modify \
it under the terms of the GNU General Public License as published by \
the Free Software Foundation; either version 2 of the License, or \
(at your option) any later version.\n"
