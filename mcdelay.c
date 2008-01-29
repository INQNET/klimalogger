/*
Copyright (C) 2000-2002 José Roberto B. de A. Monteiro <jrm@sel.eesc.usp.br>
                        and Pedro Zorzenon Neto <pzn@vztech.com.br>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Do not forget to visit Free Software Foundation site: http://fsf.org

$Id: mcdelay.c,v 1.2 2003/06/30 17:20:25 pzn Exp $
*/

/*
 From http://www.linuxdoc.org/HOWTO/mini/IO-Port-Programming.html
 an outb to port 0x80 gives a delay of aprox 1microsec independent
 of your machine type/clock.
 See also documentation in asm/io.h
*/

#include "mcdelay.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/types.h>

int old_port_value;
unsigned int m_f;

void microdelay_init(unsigned int multiply_factor) {
    /* permission to write to port 0x80
       nothing is suposed to use this port
       however, we will write the same value we've read... */
    ioperm(0x80,1,1);
    old_port_value=inb(0x80);
    m_f = multiply_factor;
}

void microdelay(unsigned int microsec) {
  unsigned long int i;
  i = m_f * microsec;
  while (i)
    {
      outb(old_port_value,0x80);
      i--;
    }
}
