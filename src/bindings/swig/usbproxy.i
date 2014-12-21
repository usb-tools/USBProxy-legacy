/* -*- c++ -*- */

//#define USBPROXY_API

%module usbproxy
%{
#include "Manager.h"
#include "ConfigParser.h"
#include "PacketFilter_Callback.h"
%}
%include "std_string.i"

%include "Manager.h"
%include "ConfigParser.h"
%include "Packet.h"
%include "PacketFilter_Callback.h"

// TODO
// Write C++ funcs with appropriate signature to wrap python functions
//%extend PacketFilter_Callback {
///* This function matches the prototype of the normal C++ callback function
// * However, the lang_* pointers hold a reference to a Python callable object
// */
//	void PythonFilterCallBack(Packet* pkt)
//	{
//		PyObject *func, *arglist;
//		func = (PyObject *) $self->lang_cb;
//		// map argument list
//		arglist = NULL; //Py_BuildValue("(d)",a);
//		PyEval_CallObject(func, arglist);
//		Py_DECREF(arglist);
//	}
//	
//	void SetCallbacks(PyObject *func) {
//		$self->lang_cb = (void *) func;
//		$self->cb = $self->PythonFilterCallBack;
//	}
//}

// Add a new helper function to register callbacks and plugin

