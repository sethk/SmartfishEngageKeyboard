/*
 *  SmartfishEngageKeyboardUSB.cpp
 *  SmartfishEngageKeyboard
 *
 *  Created by Seth Kingsley on 2/26/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "SmartfishEngageKeyboardUSB.h"

#define super IOUSBHIDDriver

OSDefineMetaClassAndStructors(SmartfishEngageKeyboardUSB, super);

bool
SmartfishEngageKeyboardUSB::init(OSDictionary *properties)
{
	if (!super::init(properties))
		return false;

	printf("%s\n", __func__);
	return true;
}

IOReturn
SmartfishEngageKeyboardUSB::handleReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options)
{
	IOLog("handleReport()\n");
	return super::handleReport(report, reportType, options);
}