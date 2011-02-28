#include "SmartfishEngageKeyboard.h"
#include <IOKit/hid/IOHIDUsageTables.h>

#define super IOHIDEventDriver

OSDefineMetaClassAndStructors(SmartfishEngageKeyboard, super);

bool
SmartfishEngageKeyboard::init(OSDictionary *properties)
{
	IOLog("Smartfish Engage Keyboard init\n");
	return super::init(properties);
}

bool
SmartfishEngageKeyboard::start(IOService *provider)
{
	IOLog("Smartfish Engage Keyboard start\n");
	return super::start(provider);
}

void
SmartfishEngageKeyboard::stop(IOService *provider)
{
	IOLog("Smartfish Engage Keyboard stop\n");
	super::stop(provider);
}

void
SmartfishEngageKeyboard::free(void)
{
	IOLog("Smartfish Engage Keyboard free\n");
	super::free();
}

IOReturn
SmartfishEngageKeyboard::message(UInt32 type, IOService *provider, void *argument)
{
	IOLog("Smartfish Engage Keyboard message\n");
	return super::message(type, provider, argument);
}

void
SmartfishEngageKeyboard::dispatchKeyboardEvent(AbsoluteTime                timeStamp,
											  UInt32                      usagePage,
											  UInt32                      usage,
											  UInt32                      value,
											  IOOptionBits                options)
{
	IOLog("dispatchKeyboardEvent(..., %lu, %lu, %lu, 0x%lx)\n", usagePage, usage, value, options);

	if (usagePage == kHIDPage_KeyboardOrKeypad)
	{
		if (usage < kHIDUsage_KeyboardLeftControl)
			--usage;

		switch (usage)
		{
			case kHIDUsage_KeyboardLeftAlt: usage = kHIDUsage_KeyboardLeftGUI; break;
			case kHIDUsage_KeyboardLeftGUI: usage = kHIDUsage_KeyboardLeftAlt; break;
			case kHIDUsage_KeyboardRightAlt: usage = kHIDUsage_KeyboardRightGUI; break;
			case kHIDUsage_KeyboardRightGUI: usage = kHIDUsage_KeyboardRightAlt; break;
		}
	}

	super::dispatchKeyboardEvent(timeStamp, usagePage, usage, value, options);
}


void
SmartfishEngageKeyboard::handleInterruptReport(AbsoluteTime                timeStamp,
											   IOMemoryDescriptor *        report,
											   IOHIDReportType             reportType,
											   UInt32                      reportID)
{
	IOLog("handleInterruptReport(..., %d, %lu)\n", reportType, reportID);

	OSArray *elements = super::getReportElements();
	if (elements)
	{
		UInt32 numElements = elements->getCount();
		IOLog("elements count = %ld\n", numElements);
		for (UInt32 elementIndex = 0; elementIndex < numElements; ++elementIndex)
		{
			IOHIDElement *element = (IOHIDElement *)elements->getObject(elementIndex);
			if (element && element->getType() != kIOHIDElementTypeCollection && element->getValue())
				IOLog("element %ld: %ld, %ld, %ld, %ld, %ld\n",
						elementIndex, element->getUsagePage(), element->getUsage(), element->getValue(),
						element->getLogicalMin(), element->getLogicalMax());
		}
	}

	super::handleInterruptReport(timeStamp, report, reportType, reportID);
}

#if 0
void
SmartfishEngageKeyboard::keyboardEvent(unsigned eventType,
						   /* flags */            unsigned flags,
						   /* keyCode */          unsigned keyCode,
						   /* charCode */         unsigned charCode,
						   /* charSet */          unsigned charSet,
						   /* originalCharCode */ unsigned origCharCode,
						   /* originalCharSet */  unsigned origCharSet)
{
	IOLog("keyboardEvent\n");
	super::keyboardEvent(eventType, flags, keyCode, charCode, charSet, origCharCode, origCharSet);
}

const unsigned char *
SmartfishEngageKeyboard::defaultKeymapOfLength(UInt32 *length)
{
	static const unsigned char kKeymap[] =
	{
		0x0, 0x0, // Data is in chars
		0x0,      // Number of modifier keys
		0x1,      // Number of key definitions
		// Key definitions:
		0xff,
		0x0,      // Number of sequences
		0x1,      // Number of special keys
		// Special Keys:
		NX_POWER_KEY, 0x1
	};

	*length = sizeof(kKeymap) / sizeof(kKeymap[0]);
	return kKeymap;
}

void
SmartfishEngageKeyboard::setAlphaLockFeedback(bool val)
{
	IOLog("setAlphaLockFeedback\n");
	super::setAlphaLockFeedback(val);
}

void
SmartfishEngageKeyboard::setNumLockFeedback(bool val)
{
	IOLog("setNumLockFeedback\n");
	super::setNumLockFeedback(val);
}

UInt32
SmartfishEngageKeyboard::maxKeyCodes()
{
	IOLog("maxKeyCodes()\n");
	return super::maxKeyCodes();
}

void
SmartfishEngageKeyboard::dispatchKeyboardEvent(unsigned int keyCode, bool goingDown, AbsoluteTime time)
{
	IOLog("%s(%u, %d, ...)\n", __func__, keyCode, (int)goingDown);
	super::dispatchKeyboardEvent(keyCode, goingDown, time);
}
#endif // 0
