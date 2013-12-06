/*
 * Copyright 2013 Dominic Spill
 * Copyright 2013 Adam Stasiak
 *
 * This file is part of USBProxy.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
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
 * PacketFilter_KeyLoggger.cpp
 *
 * Created on: Nov 23, 2013
 */
#include "PacketFilter_KeyLogger.h"
#include <memory.h>

PacketFilter_KeyLogger::PacketFilter_KeyLogger(FILE* _file) {
	file=_file;
	keyMap[0x04]='a';
	keyMap[0x05]="b";
	keyMap[0x06]="c";
	keyMap[0x07]="d";
	keyMap[0x08]="e";
	keyMap[0x09]="f";
	keyMap[0x0a]="g";
	keyMap[0x0b]="h";
	keyMap[0x0c]="i";
	keyMap[0x0d]="j";
	keyMap[0x0e]="k";
	keyMap[0x0f]="l";
	keyMap[0x10]="m";
	keyMap[0x11]="n";
	keyMap[0x12]="o";
	keyMap[0x13]="p";
	keyMap[0x14]="q";
	keyMap[0x15]="r";
	keyMap[0x16]="s";
	keyMap[0x17]="t";
	keyMap[0x18]="u";
	keyMap[0x19]="v";
	keyMap[0x1a]="w";
	keyMap[0x1b]="x";
	keyMap[0x1c]="y";
	keyMap[0x1d]="z";
	keyMap[0x1e]="1";
	keyMap[0x1f]="2";
	keyMap[0x20]="3";
	keyMap[0x21]="4";
	keyMap[0x22]="5";
	keyMap[0x23]="6";
	keyMap[0x24]="7";
	keyMap[0x25]="8";
	keyMap[0x26]="9";
	keyMap[0x27]="0";
	keyMap[0x28]="\n";
	keyMap[0x29]="{ESC}";
	keyMap[0x2a]="\b \b";
	keyMap[0x2b]="\t";
	keyMap[0x2c]=" ";
	keyMap[0x2d]="-";
	keyMap[0x2e]="=";
	keyMap[0x2f]="[";
	keyMap[0x30]="]";
	keyMap[0x31]="\\";
	keyMap[0x33]=";";
	keyMap[0x34]="\'";
	keyMap[0x35]="`";
	keyMap[0x36]=",";
	keyMap[0x37]=".";
	keyMap[0x38]="/";
	keyMap[0x39]="{CAPS LOCK}";
	keyMap[0x3A]="{F1}";
	keyMap[0x3b]="{F2}";
	keyMap[0x3c]="{F3}";
	keyMap[0x3d]="{F4}";
	keyMap[0x3e]="{F5}";
	keyMap[0x3f]="{F6}";
	keyMap[0x40]="{F7}";
	keyMap[0x41]="{F8}";
	keyMap[0x42]="{F9}";
	keyMap[0x43]="{F10}";
	keyMap[0x44]="{F11}";
	keyMap[0x45]="{F12}";
	keyMap[0x46]="{PRT SCR}";
	keyMap[0x47]="{SCR LOCK}";
	keyMap[0x48]="{PAUSE}";
	keyMap[0x49]="{INSERT}";
	keyMap[0x4A]="{HOME}";
	keyMap[0x4b]="{PG UP}";
	keyMap[0x4c]="{DELETE}";
	keyMap[0x4d]="{END}";
	keyMap[0x4e]="{PG DOWN}";
	keyMap[0x4f]="\033[1C";
	keyMap[0x50]="\033[1D";
	keyMap[0x51]="\033[1B";
	keyMap[0x52]="\033[1A";
	keyMap[0x53]="{NUM LOCK}";
	keyMap[0x54]="/";
	keyMap[0x55]="*";
	keyMap[0x56]="-";
	keyMap[0x57]="+";
	keyMap[0x59]="1";
	keyMap[0x5a]="2";
	keyMap[0x5b]="3";
	keyMap[0x5c]="4";
	keyMap[0x5d]="5";
	keyMap[0x5e]="6";
	keyMap[0x5f]="7";
	keyMap[0x60]="8";
	keyMap[0x61]="9";
	keyMap[0x62]="0";
	keyMap[0x64]="\\";
	keyMap[0x65]="{PROPERTIES}";
	shiftKeyMap[0x04]="A";
	shiftKeyMap[0x05]="B";
	shiftKeyMap[0x06]="C";
	shiftKeyMap[0x07]="D";
	shiftKeyMap[0x08]="E";
	shiftKeyMap[0x09]="F";
	shiftKeyMap[0x0a]="G";
	shiftKeyMap[0x0b]="H";
	shiftKeyMap[0x0c]="I";
	shiftKeyMap[0x0d]="J";
	shiftKeyMap[0x0e]="K";
	shiftKeyMap[0x0f]="L";
	shiftKeyMap[0x10]="M";
	shiftKeyMap[0x11]="N";
	shiftKeyMap[0x12]="O";
	shiftKeyMap[0x13]="P";
	shiftKeyMap[0x14]="Q";
	shiftKeyMap[0x15]="R";
	shiftKeyMap[0x16]="S";
	shiftKeyMap[0x17]="T";
	shiftKeyMap[0x18]="U";
	shiftKeyMap[0x19]="V";
	shiftKeyMap[0x1a]="W";
	shiftKeyMap[0x1b]="X";
	shiftKeyMap[0x1c]="Y";
	shiftKeyMap[0x1d]="Z";
	shiftKeyMap[0x1e]="!";
	shiftKeyMap[0x1f]="@";
	shiftKeyMap[0x20]="#";
	shiftKeyMap[0x21]="$";
	shiftKeyMap[0x22]="%";
	shiftKeyMap[0x23]="^";
	shiftKeyMap[0x24]="&";
	shiftKeyMap[0x25]="*";
	shiftKeyMap[0x26]="(";
	shiftKeyMap[0x27]=")";
	shiftKeyMap[0x2d]="_";
	shiftKeyMap[0x2e]="+";
	shiftKeyMap[0x2f]="{";
	shiftKeyMap[0x30]="}";
	shiftKeyMap[0x31]="|";
	shiftKeyMap[0x33]=":";
	shiftKeyMap[0x34]="\"";
	shiftKeyMap[0x35]="~";
	shiftKeyMap[0x36]="<";
	shiftKeyMap[0x37]=">";
	shiftKeyMap[0x38]="\?";
	shiftKeyMap[0x64]="|";
}

void PacketFilter_KeyLogger::keyPressed(__u8 keyCode,__u8 mods) {
	if (keyCode) {
		if (mods & 0x01) { fprintf(file,"\033[0;31m{LCTRL}\033[0m"); }
		if (mods & 0x04) { fprintf(file,"\033[0;31m{LALT}\033[0m"); }
		if (mods & 0x08) { fprintf(file,"\033[0;31m{LWIN}\033[0m"); }
		if (mods & 0x10) { fprintf(file,"\033[0;31m{RCTRL}\033[0m"); }
		if (mods & 0x40) { fprintf(file,"\033[0;31m{RALT}\033[0m"); }
		if (mods & 0x80) { fprintf(file,"\033[0;31m{RWIN}\033[0m"); }
		if ((mods & 0x22) && shiftKeyMap[keyCode]) {
			if (shiftKeyMap[keyCode][0]=='{' && shiftKeyMap[keyCode][1]!=0) {
				fprintf(file,"\033[0;34m%s\033[0m",shiftKeyMap[keyCode]);
			} else {
				fprintf(file,"%s",shiftKeyMap[keyCode]);
			}
		} else if (keyMap[keyCode]) {
			if (keyMap[keyCode][0]=='{' && keyMap[keyCode][1]!=0) {
				fprintf(file,"\033[0;34m%s\033[0m",keyMap[keyCode]);
			} else {
				fprintf(file,"%s",keyMap[keyCode]);
			}
		} else {
			fprintf(file,"{INVALID}");
		}
	} else {
		if (mods & 0x01) { fprintf(file,"\033[0;31m{LCTRL}\033[0m"); }
		if (mods & 0x02) { fprintf(file,"\033[0;31m{LSHIFT}\033[0m"); }
		if (mods & 0x04) { fprintf(file,"\033[0;31m{LALT}\033[0m"); }
		if (mods & 0x08) { fprintf(file,"\033[0;31m{LWIN}\033[0m"); }
		if (mods & 0x10) { fprintf(file,"\033[0;31m{RCTRL}\033[0m"); }
		if (mods & 0x20) { fprintf(file,"\033[0;31m{RSHIFT}\033[0m"); }
		if (mods & 0x40) { fprintf(file,"\033[0;31m{RALT}\033[0m"); }
		if (mods & 0x80) { fprintf(file,"\033[0;31m{RWIN}\033[0m"); }
	}
}

void PacketFilter_KeyLogger::filter_packet(Packet* packet) {
	int i,j;
	bool found=false,noKeys=true;
	__u8 newMods=packet->data[0]&(~lastReport[0]);
	for(i=2;i<8;i++) {
		found=false;
		for(j=0;j<8;j++) {
			if (packet->data[i]==lastReport[j]) {found=true;break;}
		}
		if (!found) {keyPressed(packet->data[i],packet->data[0]);noKeys=false;}
	}
	if (noKeys) {
		keyPressed(0,newMods);
	}
	memcpy(lastReport,packet->data,8);
}
