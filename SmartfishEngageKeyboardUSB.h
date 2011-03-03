/*
 *  SmartfishEngageKeyboardUSB.h
 *  SmartfishEngageKeyboard
 *
 *  Created by Seth Kingsley on 2/26/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <IOKit/usb/IOUSBHIDDriver.h>

class SmartfishEngageKeyboardUSB : public IOUSBHIDDriver
{
	OSDeclareDefaultStructors(SmartfishEngageKeyboardUSB);

public:
	virtual bool init(OSDictionary *properties = 0);

protected:
	virtual IOReturn handleReport(IOMemoryDescriptor *report,
								  IOHIDReportType reportType = kIOHIDReportTypeInput,
								  IOOptionBits options = 0);
};