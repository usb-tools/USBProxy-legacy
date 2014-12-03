/*
 * This file is part of USBProxy.
 */

#ifndef USBPROXY_DEFINITIONERRORS_H
#define USBPROXY_DEFINITIONERRORS_H

#include <linux/types.h>

//0xff0000   object num2
//0xff0000   object num
//0xff00 object type
//0xff     error

enum de_obj_type {
	DE_OBJ_NONE=			0x00,
	DE_OBJ_DEVICE=			0x01,
	DE_OBJ_CONFIG=			0x02,
	DE_OBJ_STRING= 			0x03,
	DE_OBJ_INTERFACE=		0x04,
	DE_OBJ_ENDPOINT=		0x05,
	DE_OBJ_QUALIFIER=		0x06,
	DE_OBJ_OS_CONFIG=		0x07,
	DE_OBJ_INTERFACEGROUP=	0x84
};

enum de_error {
	DE_ERR_NONE=				0x00,
	DE_ERR_INVALID_DESCRIPTOR=	0x01,	//detail contains 1-based byte index of error
	DE_ERR_NULL_OBJECT=			0x02,	//no detail
	DE_ERR_MISSING_LANGUAGE=	0x03,	//detail conains missing languageId, configId contains the relevant stringId
	DE_ERR_MISPLACED_OBJECT=	0x04, 	//detail contains incorrect index
	DE_ERR_MISPLACED_IF_NUM=	0x05 	//detail contains incorrect index
};

struct definition_error {
	de_error error;
	__u16 error_detail;
	de_obj_type objectType;
	__u8 configId;			//also holds stringid, high bit 1 for other speed configs
	__u8 interfaceNum;
	__u8 interfaceAlternate;
	__u8 endpointNum;
	definition_error() :
		error(DE_ERR_NONE),
		error_detail(0),
		objectType(DE_OBJ_NONE),
		configId(0),
		interfaceNum(0),
		interfaceAlternate(0),
		endpointNum(0) {}
	definition_error(de_error _error,__u16 _error_detail,de_obj_type _objectType,
			__u8 _configId=0,__u8 _interfaceNum=0,__u8 _interfaceAlternate=0,__u8 _endpointNum=0) :
		error(_error),
		error_detail(_error_detail),
		objectType(_objectType),
		configId(_configId),
		interfaceNum(_interfaceNum),
		interfaceAlternate(_interfaceAlternate),
		endpointNum(_endpointNum) {}
};

#endif /* USBPROXY_DEFINITIONERRORS_H */
