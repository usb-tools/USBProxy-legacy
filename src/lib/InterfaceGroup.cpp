/*
 * This file is part of USBProxy.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "TRACE.h"
#include "HexString.h"

#include "InterfaceGroup.h"

#include "Device.h"
#include "Interface.h"

InterfaceGroup::InterfaceGroup(__u8 _number) {
	number=_number;
	alternateCount=0;
	activeAlternateIndex=-1;
	interfaces=NULL;
}
InterfaceGroup::~InterfaceGroup() {
	if (interfaces) {
		int i;
		for(i=0;i<alternateCount;i++) {
			if (interfaces[i]) {
				delete(interfaces[i]);
				interfaces[i]=NULL;
			}
		}
		free(interfaces);
		interfaces=NULL;
	}
}

size_t InterfaceGroup::get_full_descriptor_length() {
	size_t total=0;
	int i;
	for(i=0;i<alternateCount;i++) {
		// modified 20140903 atsumi@aizulab.com
		if ( interfaces[i]) {
			total+=interfaces[i]->get_full_descriptor_length();
		}
	}
	return total;
}

void InterfaceGroup::get_full_descriptor(__u8** p) {
	int i;
	
	for(i=0;i<alternateCount;i++) {
		// modified 20140903 atsumi@aizulab.com
		if ( interfaces[i]) {
			interfaces[i]->get_full_descriptor(p);
		}
	}
}

void InterfaceGroup::add_interface(Interface* interface) {
	__u8 alternate=interface->get_descriptor()->bAlternateSetting;
	if (alternate>=alternateCount) {
		Interface** newInterfaces=(Interface **)calloc(alternate+1,sizeof(*interfaces));
		if (alternateCount) {
			memcpy(newInterfaces,interfaces,sizeof(*interfaces)*alternateCount);
			free(interfaces);
			/* not needed interaces=NULL; */
		}
		interfaces=newInterfaces;
		alternateCount=alternate+1;
	} else {
		if (interfaces[alternate]) {delete(interfaces[alternate]);/* not needed interfaces[alternate]=NULL;*/}
	}
	interfaces[alternate]=interface;
}

Interface* InterfaceGroup::get_interface(__u8 alternate) {
	if (alternate>=alternateCount || alternate<0) {return NULL;}
	return interfaces[alternate];
}

void InterfaceGroup::print(__u8 tabs) {
	int i;
	printf("%.*sInterface(%d):\n",tabs,TABPADDING,number);
	for(i=0;i<alternateCount;i++) {
		// modified 20140903 atsumi@aizulab.com
		if ( interfaces[i]) {
			interfaces[i]->print(tabs+1,i==activeAlternateIndex?true:false);
		}
	}
}

__u8 InterfaceGroup::get_number() {
	return number;
}

__u8 InterfaceGroup::get_alternate_count() {
	return alternateCount;
}

Interface* InterfaceGroup::get_active_interface() {
	if (activeAlternateIndex<0) {return NULL;}
	return get_interface(activeAlternateIndex);
}

const definition_error InterfaceGroup::is_defined(__u8 configId) {
	if (!alternateCount) {return definition_error(DE_ERR_NULL_OBJECT,0x0, DE_OBJ_INTERFACE,configId,number,0);}
	int i;
	for(i=0;i<alternateCount;i++) {
		if (!interfaces[i]) {return definition_error(DE_ERR_NULL_OBJECT,0x0, DE_OBJ_INTERFACE,configId,number,i);}
		if (interfaces[i]->get_descriptor()->bInterfaceNumber!=number) {return definition_error(DE_ERR_MISPLACED_IF_NUM,interfaces[i]->get_descriptor()->bAlternateSetting, DE_OBJ_INTERFACE,configId,number,i);}
		if (interfaces[i]->get_descriptor()->bAlternateSetting!=i) {return definition_error(DE_ERR_MISPLACED_OBJECT,interfaces[i]->get_descriptor()->bAlternateSetting, DE_OBJ_INTERFACE,configId,number,i);}
		definition_error rc=interfaces[i]->is_defined(configId,number);
		if (rc.error) {return rc;}
	}
	return definition_error();
}
