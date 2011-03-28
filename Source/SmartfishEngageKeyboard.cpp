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

#define super IOHIDEventDriver

OSDefineMetaClassAndStructors(SmartfishEngageKeyboard, super);

static const struct {u_char usb, adb;} kVirtualFKeys[] =
{
	{kHIDUsage_KeyboardNonUSBackslash /* USB 0x64 */, 0x0a},
	{kHIDUsage_KeyboardPower /* USB 0x66 */, 0x7f},
	{kHIDUsage_KeyboardF18 /* USB 0x6d */, 0x4f},
	{kHIDUsage_KeyboardF19 /* USB 0x6e */, 0x50},
	{kHIDUsage_KeyboardF20 /* USB 0x6f */, 0x5a},
	{kHIDUsage_KeypadComma /* USB 0x85 */, 0x5f},
	{kHIDUsage_KeyboardVolumeUp /* USB 0x80 */, 0x48},
	{kHIDUsage_KeyboardVolumeDown /* USB 0x81 */, 0x49},
	{kHIDUsage_KeyboardInternational1 /* USB 0x87 */, 0x5e},
	{kHIDUsage_KeyboardInternational3 /* USB 0x89 */, 0x5d},
	{kHIDUsage_KeyboardLANG1 /* USB 0x90 */, 0x68},
	{kHIDUsage_KeyboardLANG2 /* USB 0x91 */, 0x66},
	{kHIDUsage_KeyboardMute /* USB 0x7f */, 0x4a}
};
static const u_char kNumVirtualFKeys = sizeof(kVirtualFKeys) / sizeof(*kVirtualFKeys);

enum
{
	EX_KEYTYPE_EXPOSE = NX_NUMSPECIALKEYS,
	EX_KEYTYPE_DASHBOARD
};

bool
SmartfishEngageKeyboard::init(OSDictionary *properties)
{
	if (super::init(properties))
	{
		memset(_fKeyMappings, (u_char)NX_NOSPECIALKEY, sizeof(_fKeyMappings));
		_fKeyMappings[kHIDUsage_KeyboardF1] = NX_KEYTYPE_BRIGHTNESS_DOWN;
		_fKeyMappings[kHIDUsage_KeyboardF2] = NX_KEYTYPE_BRIGHTNESS_UP;
		_fKeyMappings[kHIDUsage_KeyboardF3] = EX_KEYTYPE_EXPOSE;
		_fKeyMappings[kHIDUsage_KeyboardF4] = EX_KEYTYPE_DASHBOARD;
		_fKeyMappings[kHIDUsage_KeyboardF7] = NX_KEYTYPE_PREVIOUS;
		_fKeyMappings[kHIDUsage_KeyboardF8] = NX_KEYTYPE_PLAY;
		_fKeyMappings[kHIDUsage_KeyboardF9] = NX_KEYTYPE_NEXT;
		_fKeyMappings[kHIDUsage_KeyboardF10] = NX_KEYTYPE_MUTE;
		_fKeyMappings[kHIDUsage_KeyboardF11] = NX_KEYTYPE_SOUND_DOWN;
		_fKeyMappings[kHIDUsage_KeyboardF12] = NX_KEYTYPE_SOUND_UP;
#if 0
		_fKeyMappings[kHIDUsage_KeyboardDeleteOrBackspace] = NX_KEYTYPE_EJECT;
#endif // 0

		return true;
	}
	else
		return false;
}

IOReturn
SmartfishEngageKeyboard::setSystemProperties(OSDictionary *properties)
{
#include "KeyMap.cpp"

	IOReturn ret = kIOReturnBadArgument;
	if (properties)
	{
		OSDictionary *propsCopy = OSDictionary::withDictionary(properties);
		OSData *keyMapData = OSData::withBytes(kKeyMap, sizeof(kKeyMap));

		keyMapData->appendByte('\0', 1);
		u_char numFKeyMappings = 0;
		bzero(_fKeyVirtualMappings, sizeof(_fKeyVirtualMappings));
		for (u_short usage = 0; usage < NX_NUMKEYCODES; ++usage)
			switch (_fKeyMappings[usage])
			{
				case (u_char)NX_NOSPECIALKEY: continue;

				case NX_KEYTYPE_BRIGHTNESS_DOWN: _fKeyVirtualMappings[usage] = kHIDUsage_KeyboardF14; break;
				case NX_KEYTYPE_BRIGHTNESS_UP: _fKeyVirtualMappings[usage] = kHIDUsage_KeyboardF15; break;
				case EX_KEYTYPE_DASHBOARD: _fKeyVirtualMappings[usage] = kHIDUsage_KeyboardF16; break;
				case EX_KEYTYPE_EXPOSE: _fKeyVirtualMappings[usage] = kHIDUsage_KeyboardF17; break;

				default:
					if (numFKeyMappings < kNumVirtualFKeys)
					{
						_fKeyVirtualMappings[usage] = kVirtualFKeys[numFKeyMappings].usb;
						keyMapData->appendByte(_fKeyMappings[usage], 1);
						keyMapData->appendByte(kVirtualFKeys[numFKeyMappings].adb, 1);
						++numFKeyMappings;
					}
					break;
			}
		u_char *pCount = (u_char *)keyMapData->getBytesNoCopy(sizeof(kKeyMap), 1);
		*pCount = numFKeyMappings;

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
		else if (_fKeyState && _fKeyMappings[usage] != (u_char)NX_NOSPECIALKEY)
			usage = _fKeyVirtualMappings[usage];
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
