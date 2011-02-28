#include <IOKit/hidevent/IOHIDEventDriver.h>

class SmartfishEngageKeyboard : public IOHIDEventDriver
{
	OSDeclareDefaultStructors(SmartfishEngageKeyboard);

public:
	virtual bool init(OSDictionary *properties = 0);
	virtual bool start(IOService *provider);
	virtual void stop(IOService *provider);
	virtual void free(void);
	virtual IOReturn message(UInt32 type, IOService *provider, void *argument = 0);

protected:
	virtual void            dispatchKeyboardEvent(
												  AbsoluteTime                timeStamp,
												  UInt32                      usagePage,
												  UInt32                      usage,
												  UInt32                      value,
												  IOOptionBits                options = 0 );

	virtual void            handleInterruptReport (
												   AbsoluteTime                timeStamp,
												   IOMemoryDescriptor *        report,
												   IOHIDReportType             reportType,
												   UInt32                      reportID);	
#if 0
	virtual void keyboardEvent(unsigned eventType,
							   /* flags */            unsigned flags,
							   /* keyCode */          unsigned keyCode,
							   /* charCode */         unsigned charCode,
							   /* charSet */          unsigned charSet,
							   /* originalCharCode */ unsigned origCharCode,
							   /* originalCharSet */  unsigned origCharSet);	

protected:
	virtual const unsigned char * defaultKeymapOfLength(UInt32 *length);
	virtual void setAlphaLockFeedback(bool val);
	virtual void setNumLockFeedback(bool val);
	virtual UInt32 maxKeyCodes();
	virtual void dispatchKeyboardEvent(unsigned int keyCode, bool goingDown, AbsoluteTime time);
#endif // 0
};