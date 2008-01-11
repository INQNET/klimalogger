/*
Copyright (C) 2000-2002 Jos� Roberto B. de A. Monteiro <jrm@sel.eesc.usp.br>
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

$Id: mcdelay.h,v 1.2 2003/06/30 17:20:25 pzn Exp $
*/

#ifndef _MICRODELAY_H
#define _MICRODELAY_H

void microdelay_init(unsigned int multiply_factor);
void microdelay(unsigned int microsec);

#endif
