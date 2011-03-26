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
#include <IOKit/hid/AppleHIDUsageTables.h>
#include <IOKit/hidsystem/ev_keymap.h>

#define super IOHIDEventDriver

OSDefineMetaClassAndStructors(SmartfishEngageKeyboard, super);

static const u_char kVirtualFKeys[] =
{
	kHIDUsage_KeyboardNonUSBackslash, /* USB 0x64, ADB 0x0a */
	kHIDUsage_KeyboardPower,          /* USB 0x66, ADB 0x7f */
	kHIDUsage_KeypadComma,            /* USB 0x85, ADB 0x5f */
	kHIDUsage_KeyboardInternational1, /* USB 0x87, ADB 0x5e */
	kHIDUsage_KeyboardInternational3, /* USB 0x89, ADB 0x5d */
	kHIDUsage_KeyboardF18,            /* USB 0x6d, ADB 0x4f */
	(u_char)NX_NOSPECIALKEY,
	(u_char)NX_NOSPECIALKEY,
	kHIDUsage_KeyboardF19,            /* USB 0x6e, ADB 0x50 */
	kHIDUsage_KeyboardF20,            /* USB 0x6f, ADB 0x5a */
	kHIDUsage_KeyboardLANG2,          /* USB 0x91, ADB 0x66 */
	kHIDUsage_KeyboardMute,           /* USB 0x7f, ADB 0x4a */
	kHIDUsage_KeyboardVolumeDown,     /* USB 0x81, ADB 0x49 */
	kHIDUsage_KeyboardVolumeUp        /* USB 0x80, ADB 0x48 */
};

static const u_char kFKeyMappings[] =
{
	NX_KEYTYPE_BRIGHTNESS_DOWN,
	NX_KEYTYPE_BRIGHTNESS_UP,
	(u_char)NX_NOSPECIALKEY, /* Exposé */
	(u_char)NX_NOSPECIALKEY, /* Dashboard */
	(u_char)NX_NOSPECIALKEY,
	(u_char)NX_NOSPECIALKEY,
	NX_KEYTYPE_PREVIOUS,
	NX_KEYTYPE_PLAY,
	NX_KEYTYPE_NEXT,
	NX_KEYTYPE_MUTE,
	NX_KEYTYPE_SOUND_DOWN,
	NX_KEYTYPE_SOUND_UP
};

IOReturn
SmartfishEngageKeyboard::setSystemProperties(OSDictionary *properties)
{
#include "KeyMap.cpp"

	IOReturn ret = kIOReturnBadArgument;
	if (properties)
	{
		OSDictionary *propsCopy = OSDictionary::withDictionary(properties);
		OSData *keyMapData = OSData::withBytes(kKeyMap, sizeof(kKeyMap));
		propsCopy->setObject(kIOHIDKeyMappingKey, keyMapData);
		keyMapData->release();
#ifdef DEBUG
		IOLog("setSystemProperties(): updating keymap, HIDDeviceParameters = %p\n",
				propsCopy->getObject("HIDDeviceParameters"));
#endif // DEBUG
		ret = super::setSystemProperties(propsCopy);
		propsCopy->release();

		if (properties->getObject(kIOHIDResetKeyboardKey))
			_fKeyState = 0;
	}

	return ret;
}

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
		// Compensate for difference in interpretation of minValue = 1 between OS X and other OSes:
		if (0 < usage < kHIDUsage_KeyboardLeftControl)
			--usage;

		if (usage == kHIDUsage_KeyboardApplication)
			_fKeyState = (value != 0);
		else if (_fKeyState && kHIDUsage_KeyboardF1 <= usage <= kHIDUsage_KeyboardF12 &&
				kFKeyMappings[usage - kHIDUsage_KeyboardF1] != (u_char)NX_NOSPECIALKEY)
			usage = kVirtualFKeys[usage - kHIDUsage_KeyboardF1];
#ifdef DEBUG
		else switch (usage)
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
