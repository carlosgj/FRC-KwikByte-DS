#
# Sample ADC Program for KB9260 and related products
#
# Copyright (C) 2009 KwikByte <www.kwikbyte.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# or visit http://www.gnu.org/copyleft/gpl.html

# 22MAY2009 	Initial creation (KwikByte)

BASE_TOOLS=/buildroot/build_arm_nofpu/staging_dir/bin
CC=${BASE_TOOLS}/arm-linux-gcc
LD=${BASE_TOOLS}/arm-linux-ld

LDFLAGS=-static -g

.PHONY: all

all:: adc_app

adc_app: main.o
	${CC} ${LDFLAGS} $? -o $@ ${LIBS} ${EXTRALIBS}

.c.o:
	${CC} ${CFLAGS} -c $<

cleandir::
	$(RM) *.o adc_app

clean:: cleandir

distclean:: cleandir

