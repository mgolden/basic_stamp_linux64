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


enum module_type {None, Reserved, bs2, bs2e, bs2sx, bs2p, bs2pe};


/* Interface Functions */

void p_init(const char *devname);
int p_term(void);
int p_reset(void);
enum module_type p_ident(char *type, char *version);
int p_proginit(int progslot);
int p_progblast( void *buff);
int p_complete(void);
int p_halt(void);
void p_deinit(void);
int returnfd(void);



