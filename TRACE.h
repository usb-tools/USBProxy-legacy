/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 *
 * This file is part of USB-MitM.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 *
 * TRACE.h
 *
 * Created on: Nov 21, 2013
 */
#ifndef TRACE_H_
#define TRACE_H_

#include <stdio.h>

#define TRACE fprintf(stderr,"Trace: %s, line %d\n",__FILE__,__LINE__);
#define TRACE1(X) fprintf(stderr,"Trace(%d): %s, line %d\n",X,__FILE__,__LINE__);
#define TRACE2(X,Y) fprintf(stderr,"Trace(%d,%d): %s, line %d\n",X,Y,__FILE__,__LINE__);
#define TRACE3(X,Y,Z) fprintf(stderr,"Trace(%d,%d,%d): %s, line %d\n",X,Y,Z,__FILE__,__LINE__);

#endif /* TRACE_H_ */
