/*-
 * Copyright (c) 2011, Seth Kingsley
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 *
 * • Redistributions of source code must retain the above copyright notice, this list of conditions and the following
 *   disclaimer.
 * • Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
 *   disclaimer in the documentation and/or other materials provided with the distribution.
 * • Neither the name Seth Kingsley nor the names of other contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#include "SmartfishEngageKeyboard.h"
#include <IOKit/hid/IOHIDUsageTables.h>

#define super IOHIDEventDriver

OSDefineMetaClassAndStructors(SmartfishEngageKeyboard, super);

void
SmartfishEngageKeyboard::dispatchKeyboardEvent(AbsoluteTime timeStamp,
		UInt32 usagePage, UInt32 usage, UInt32 value,
		IOOptionBits options)
{
#ifdef DEBUG
	IOLog("dispatchKeyboardEvent(..., %lu, %lu, %lu, 0x%lx)\n", usagePage, usage, value, options);
#endif // DEBUG

	if (usagePage == kHIDPage_KeyboardOrKeypad)
	{
		if (usage < kHIDUsage_KeyboardLeftControl)
			--usage;

#ifdef DEBUG
		switch (usage)
		{
			case kHIDUsage_KeyboardLeftAlt: usage = kHIDUsage_KeyboardLeftGUI; break;
			case kHIDUsage_KeyboardLeftGUI: usage = kHIDUsage_KeyboardLeftAlt; break;
			case kHIDUsage_KeyboardRightAlt: usage = kHIDUsage_KeyboardRightGUI; break;
			case kHIDUsage_KeyboardRightGUI: usage = kHIDUsage_KeyboardRightAlt; break;
		}
#endif // DEBUG
	}

	super::dispatchKeyboardEvent(timeStamp, usagePage, usage, value, options);
}
