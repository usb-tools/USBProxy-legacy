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
 * PacketFilter_Python.cpp
 *
 * Created on: Jan 10, 2014
 */

#include "PacketFilter_Python.h"
#include <stdio.h>

int PacketFilter_Python::debugLevel=0;

PacketFilter_Python::PacketFilter_Python(char* modulename) {
	//PyObject *p_filter_name;

	//Py_SetProgramName("USBProxy");
	Py_Initialize();
	//pymodname = PyUnicode_FromString(modulename);
    /* Error checking of pName left out */

    p_module = PyImport_ImportModule(modulename);
    //Py_DECREF(pymodname);
	if (p_module == NULL) {
		if (PyErr_Occurred())
			PyErr_Print();
        fprintf(stderr, "Failed to load '%s'\n", modulename);
        return;
	}
	
	/* Import USBProxy python module for Packet and SetupPacket */
	//pymodname = PyUnicode_FromString("USBProxy");

    p_usbproxy = PyImport_ImportModule("USBProxy");
    //Py_DECREF(pymodname);
	if (p_usbproxy == NULL) {
		if (PyErr_Occurred())
			PyErr_Print();
        fprintf(stderr, "Failed to load 'USBProxy module'\n");
        return;
	}

	p_filter_func = PyObject_GetAttrString(p_module, "filter_packet");
	if (!PyCallable_Check(p_filter_func)) {
        if (PyErr_Occurred())
            PyErr_Print();
		p_filter_func = NULL;
        fprintf(stderr, "Cannot find function 'filter_packet'\n");
		return;
    }

	p_setup_func = PyObject_GetAttrString(p_module, "filter_setup_packet");
	if (!PyCallable_Check(p_setup_func)) {
        if (PyErr_Occurred())
            PyErr_Print();
		p_setup_func = NULL;
        fprintf(stderr, "Cannot find function 'filter_setup_packet'\n");
		return;
    }

//	p_filter_name = PyObject_GetAttrString(p_module, "filter_name");
//	if (!p_filter_name) {
//        if (PyErr_Occurred())
//            PyErr_Print();
//        fprintf(stderr, "Using default filter name\n");
//		p_filter_name = PyUnicode_FromString(modulename);
//    } else {
//		
//	}
}

PacketFilter_Python::~PacketFilter_Python() {
    Py_DECREF(p_module);
    Py_DECREF(p_usbproxy);
	Py_Finalize();
}

void PacketFilter_Python::filter_packet(Packet* packet) {
	if(p_filter_func) {
		PyObject *py_pkt_cls, *py_pkt, *pkt_args, *args;
		py_pkt_cls = PyObject_GetAttrString(p_usbproxy, "Packet");

		// Create an instance of the class
		if (PyCallable_Check(py_pkt_cls))
		{
			pkt_args = Py_BuildValue("bhNNy",
									 packet->bEndpoint,
									 packet->wLength,
									 packet->filter ? Py_True:Py_False,
									 packet->transmit ? Py_True:Py_False,
									 packet->data);
			py_pkt = PyObject_CallObject(py_pkt_cls, pkt_args); 
		} else {
			fprintf(stderr, "Error: SetupPacket class not callable - skipping filter\n");
			return;
		}
		args = PyTuple_New(1);
		PyTuple_SetItem(args, 0, py_pkt);
		py_pkt = PyObject_Call(p_filter_func, args, NULL);
		
		/* Do womething with returned packet */
		
		Py_DECREF(args);
		Py_DECREF(py_pkt);
		Py_DECREF(pkt_args);
		Py_DECREF(py_pkt_cls);
	}
}

void PacketFilter_Python::filter_setup_packet(SetupPacket* packet, bool direction_out) {
	if(p_setup_func) {
		PyObject *py_pkt_cls, *py_pkt, *pkt_args, *py_dir, *args;
		py_pkt_cls = PyObject_GetAttrString(p_usbproxy, "SetupPacket");

		// Create an instance of the class
		if (PyCallable_Check(py_pkt_cls))
		{
			pkt_args = Py_BuildValue("bbhhhiNNNNy",
									 packet->ctrl_req.bRequestType,
									 packet->ctrl_req.bRequest,
									 packet->ctrl_req.wValue,
									 packet->ctrl_req.wIndex,
									 packet->ctrl_req.wLength,
									 packet->source,
									 packet->filter_out ? Py_True:Py_False,
									 packet->transmit_out ? Py_True:Py_False,
									 packet->filter_in ? Py_True:Py_False,
									 packet->transmit_in ? Py_True:Py_False,
									 packet->data);
			py_pkt = PyObject_CallObject(py_pkt_cls, pkt_args); 
		} else {
			fprintf(stderr, "Error: SetupPacket class not callable - skipping filter\n");
			return;
		}
		
		args = PyTuple_New(2);
		PyTuple_SetItem(args, 0, py_pkt);
		if(direction_out)
			py_dir = Py_True;
		else
			py_dir = Py_False;
		PyTuple_SetItem(args, 1, py_dir);
		py_pkt = PyObject_Call(p_setup_func, args, NULL);
		
		/* Do womething with returned packet */
		
		Py_DECREF(args);
		Py_DECREF(py_pkt);
		Py_DECREF(pkt_args);
		Py_DECREF(py_pkt_cls);
		Py_DECREF(py_dir);
	}
}
