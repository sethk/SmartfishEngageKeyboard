/*
 *  SmartfishEngageKeyboardUSB.h
 *  SmartfishEngageKeyboard
 *
 *  Created by Seth Kingsley on 2/26/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <IOKit/hidevent/IOHIDEventDriver.h>

class SmartfishEngageKeyboardUSB : public IOHIDEventDriver
{
	OSDeclareDefaultStructors(SmartfishEngageKeyboardUSB);

public:
	virtual bool init(OSDictionary *properties = 0);
};