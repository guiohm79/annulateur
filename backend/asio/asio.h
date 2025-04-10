//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------

#include "asiosys.h"
#include "asiotypes.h"

/*
	Steinberg Audio Stream I/O API
	(c) 1997 - 2019, Steinberg Media Technologies GmbH

	ASIO Interface Specification v 2.3

	2005 - Added support for DSD sample data (in cooperation with Sony)
	2012 - Added support for drop out detection
		
	

	basic concept is an i/o synchronous double-buffer scheme:
	
	on bufferSwitch(index == 0), host will read/write:

		after ASIOStart(), the
  read  first input buffer A (index 0)
	|   will be invalid (empty)
	*   ------------------------
	|------------------------|-----------------------|
	|                        |                       |
	|  Input Buffer A (0)    |   Input Buffer B (1)  |
	|                        |                       |
	|------------------------|-----------------------|
	|                        |                       |
	|  Output Buffer A (0)   |   Output Buffer B (1) |
	|                        |                       |
	|------------------------|-----------------------|
	*                        -------------------------
	|                        before calling ASIOStart(),
  write                      host will have filled output
                             buffer B (index 1) already

  *please* take special care of proper statement of input
  and output latencies (see ASIOGetLatencies()), these
  control sequencer sync accuracy

*/

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------

/*

prototypes summary:

ASIOError ASIOInit(ASIODriverInfo *info);
ASIOError ASIOExit(void);
ASIOError ASIOStart(void);
ASIOError ASIOStop(void);
ASIOError ASIOGetChannels(long *numInputChannels, long *numOutputChannels);
ASIOError ASIOGetLatencies(long *inputLatency, long *outputLatency);
ASIOError ASIOGetBufferSize(long *minSize, long *maxSize, long *preferredSize, long *granularity);
ASIOError ASIOCanSampleRate(ASIOSampleRate sampleRate);
ASIOError ASIOGetSampleRate(ASIOSampleRate *currentRate);
ASIOError ASIOSetSampleRate(ASIOSampleRate sampleRate);
ASIOError ASIOGetClockSources(ASIOClockSource *clocks, long *numSources);
ASIOError ASIOSetClockSource(long reference);
ASIOError ASIOGetSamplePosition (ASIOSamples *sPos, ASIOTimeStamp *tStamp);
ASIOError ASIOGetChannelInfo(ASIOChannelInfo *info);
ASIOError ASIOCreateBuffers(ASIOBufferInfo *bufferInfos, long numChannels,
	long bufferSize, ASIOCallbacks *callbacks);
ASIOError ASIODisposeBuffers(void);
ASIOError ASIOControlPanel(void);
void *ASIOFuture(long selector, void *params);
ASIOError ASIOOutputReady(void);

*/

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------

#ifndef __ASIO_H
#define __ASIO_H

#include "asiosys.h"	// system dependent layer
#include "asiotypes.h"	// platform dependent data type definitions

// force 4 byte alignment
#if defined(_MSC_VER) && !defined(__MWERKS__) 
#pragma pack(push,4)
#elif PRAGMA_ALIGN_SUPPORTED
#pragma options align = native
#endif

//- - - - - - - - - - - - - - - - - - - - - - - - -
// Type definitions are in asiotypes.h
//- - - - - - - - - - - - - - - - - - - - - - - - -

//-------------------------------------------------------------------------------------------
// ASIO Error codes are defined in asiotypes.h
//-------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
// ASIOInit() requires a pointer to the ASIODriverInfo struct
// - the name field is a C string of max 32 bytes including the
// terminator. The driver should fill it with its name.
//-------------------------------------------------------------------------------------------
typedef struct ASIODriverInfo
{
	long asioVersion;		// currently, 2
	long driverVersion;		// driver specific
	char name[32];
	char errorMessage[124];
	void *sysRef;			// must be null, system specific interface, used for additional functionality
} ASIODriverInfo;

//-------------------------------------------------------------------------------------------
// ASIOGetChannels() returns the number of available input and output channels
// - numInputChannels: address where to store the number of available input channels
// - numOutputChannels: address where to store the number of available output channels
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// ASIOGetBufferSize() returns the min/max/preferred and granularity of buffer sizes
// the driver supports. granularity is the number of frames to adjust the buffer size
// with. Should be a power of 2. If -1 is returned for granularity, the driver
// supports only the preferred buffer size.
// - minSize: address where to store the minimum buffer size in sample frames
// - maxSize: address where to store the maximum buffer size in sample frames
// - preferredSize: address where to store the preferred buffer size in sample frames
// - granularity: address where to store the granularity. See above
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// ASIOGetLatencies() returns the input and output latencies
// Latencies are expressed in sample frames.
// - inputLatency: address where to store the input latency in sample frames
// - outputLatency: address where to store the output latency in sample frames
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// ASIOGetSampleRate() returns the current sample rate
// - sampleRate: address where to store the current sample rate
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// ASIOSetSampleRate() sets the sample rate
// - sampleRate: the sample rate to be set
// Note: this is checking the sample rate only. It does not store it. This has to be
// done by the host IF the call returns ASE_SUCCESS!
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// ASIOCanSampleRate() check if the hardware is supporting a sample rate
// - sampleRate: the sample rate to be checked
// Returns ASE_SUCCESS if the sample rate is supported
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// ASIOGetClockSources() gets a list of clock sources available
// - clocks: a pointer to an array of ASIOClockSource structures. The driver will fill
//           this array with the descriptions of the available clock sources
// - numSources: on input, the number of allocated ASIOClockSource structures in the
//               array. On output, the number of clock sources the driver found.
// Use ASIOGetClockSources(NULL, &numSources) to learn how many sources are available.
// The first entry in the clocks array is always the internal clock source.
//-------------------------------------------------------------------------------------------
typedef struct ASIOClockSource
{
	long index;				// as used for ASIOSetClockSource()
	long associatedChannel;	// channel index if clock source is locked to an input channel, else -1
	long associatedGroup;		// group index if clock source is locked to a group of channels, else -1
	ASIOBool isCurrentSource;// ASIOTrue if this is the current clock source
	char name[32];			// for user selection
} ASIOClockSource;


//-------------------------------------------------------------------------------------------
// ASIOSetClockSource() set the clock source
// - index: the index of the clock source to be set
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// ASIOGetSamplePosition() gets the current sample position and time stamp
// - sPos: address where to store the current sample frame position (64 bit)
// - tStamp: address where to store the current time stamp (64 bit, in nanoseconds)
// Note: the time stamp must be coordinated with the system time; for instance,
// on the Mac, Mach micro-seconds AbsoluteTime can be used, calculated from the
// sample frame position and the sample rate.
// tStamp should be valid when the driver is running, but does not need to be
// valid when the driver is stopped.
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// ASIOGetChannelInfo() gets information about a channel
// - channelInfo: pointer to the ASIOChannelInfo structure to be filled
// Note: channel number is 0-indexed. Both inputs and outputs are using the same
// index range. For example, if a device has 2 inputs and 2 outputs, the indices
// are: input 0, input 1, output 0, output 1.
//-------------------------------------------------------------------------------------------
typedef struct ASIOChannelInfo
{
	long channel;		// on input, channel index
	ASIOBool isInput;	// on input, ASIOTrue for input channels, ASIOFalse for output channels
	ASIOBool isActive;	// on exit
	long channelGroup;	// dto. discrete group (e.g. ADAT 1-8, ..) starting at 0
	ASIOSampleType type;	// dto. sample type (see ASIOSampleType)
	char name[32];		// dto. name
} ASIOChannelInfo;

//-------------------------------------------------------------------------------------------
// ASIOCreateBuffers() creates the buffers to be used with ASIOProcess() / ASIOOutputReady()
// - bufferInfos: a pointer to an array of ASIOBufferInfo structures. ASIO will fill this
//                array with the buffer addresses for all active channels.
// - numChannels: the number of channels for which buffers should be created. If the
//                driver supports inputs and outputs, the channel indices range from
//                0 to numInputChannels + numOutputChannels - 1.
// - bufferSize: the requested buffer size in sample frames.
// - callbacks: pointer to the ASIOCallbacks structure. Needed for the bufferSwitch() call.
//-------------------------------------------------------------------------------------------
typedef struct ASIOBufferInfo
{
	ASIOBool isInput;	// on input: ASIOTrue: input, ASIOFalse: output
	long channelNum;	// on input: channel index
	void *buffers[2];	// on output: double buffer addresses
} ASIOBufferInfo;


//-------------------------------------------------------------------------------------------
// ASIODisposeBuffers() dispose the buffers created by createBuffers()
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// ASIOControlPanel() brings up the host application control panel
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// ASIOOutputReady() notifies the driver that the host has completed processing the
// output buffer. This is only used when the host is driving the device; that is,
// the driver does not initiates the buffer switches itself.
// Note: this call is only needed when the driver is in a mode where it does not
// call bufferSwitch() by itself.
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// ASIOFuture() handles future extensions or driver specific calls
// - selector: identifier of the function to be called
// - params: pointer to the parameter block belonging to the selector
// Note: This is used for functions which are not part of the standard ASIO spec.
// See the separate document "ASIO Future extensions" for details.
//-------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------
// ASIOCallbacks struct contains the addresses to the four callback functions
// bufferSwitch() and sampleRateDidChange() are always required, bufferSwitchTimeInfo()
// and asioMessage() are optional.
//-------------------------------------------------------------------------------------------
typedef struct ASIOTime // requires structure size of 24 bytes
{
	long     reserved[4];	// must be 0
	ASIOTimeInfo timeInfo;	// time info
	ASIOTimeCode timeCode;	// time code
} ASIOTime;

typedef struct ASIOCallbacks
{
	// Definition, see comments below
	// bufferSwitch() is called on a high priority kernal thread,
	// !!!! Do not allocate memory or call blocking functions !!!!
	void (*bufferSwitch) (long doubleBufferIndex, ASIOBool directProcess);

	// Definition, see comments below
	void (*sampleRateDidChange) (ASIOSampleRate sRate);

	// Definition, see comments below
	// new messages may be added from time to time
	long (*asioMessage) (long selector, long value, void* message, double* opt);

	// Definition, see comments below
	// gets called if the driver supports the ASIOOverloadDetection feature.
	ASIOTime* (*bufferSwitchTimeInfo) (ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);

} ASIOCallbacks;

/* Definition:
//-------------------------------------------------------------------------------------------
// void bufferSwitch(long doubleBufferIndex, ASIOBool directProcess);
// indicates that both input and output are to be processed.
// doubleBufferIndex: represents the actual buffer index (0 or 1). This value is to be used
//                    when calling ASIOGetChannelInfo() and ASIOGetLatencies().
// directProcess: if ASIOTrue, the host must process the buffers within this call,
//                otherwise it should schedule the processing to another thread.
// Note: bufferSwitch() is called on a high priority thread. Don't do anything
// time consuming. Never call the driver from the bufferSwitch()!
// Do not allocate memory or call blocking functions! Be careful with printf()!

//-------------------------------------------------------------------------------------------
// void sampleRateDidChange(ASIOSampleRate sRate);
// must be called by the driver when the hardware sample rate changes. This is
// only allowed when the driver is in the stopped state.

//-------------------------------------------------------------------------------------------
// long asioMessage(long selector, long value, void* message, double* opt);
// generic callback handler for various purposes. If the host supports a selector
// which is sent by the driver, it should return 1, else 0.
// Currently documented selectors:
//   kAsioSelectorSupported: if the host supports the selector passed in <value>,
//                           it should return 1.
//   kAsioEngineVersion: the driver requests the version of the engine/host it
//                       is connected to. Return 1 and put the version number
//                       into <value>.
//   kAsioResetRequest: the driver requests the host to call ASIOReset(). Return 1 if
//                      the host agrees to do so.
//   kAsioBufferSizeChangeRequest: the driver asks the host to change the buffer size
//                                to the value passed in <value>. Return 1 if
//                                the host agrees to do so.
//   kAsioResyncRequest: the driver detected a sample slip or buffer overflow.
//                     Return 1 if the host agrees to resync.
//   kAsioLatenciesChanged: the driver informs the host that latencies have changed.
//                          The host should call ASIOGetLatencies() soon.
//   kAsioSupportsTimeInfo: the driver asks the host if it supports the
//                          bufferSwitchTimeInfo callback. Return 1 if yes.
//   kAsioSupportsTimeCode: the driver asks the host if it supports the
//                          kAsioSetTimeCode ASIOFuture call. Return 1 if yes.
//   kAsioMMCCommand: the driver has received a MMC command which should be passed
//                  to the host. <value> contains the length of the MMC command in
//                  bytes. <message> contains the pointer to the MMC data.
//                  Return 1 if the host supports MMC processing.
//   kAsioSupportsInputMonitor: the driver asks the host if it supports input monitoring
//                            via the kAsioEnableTimeCodeRead and kAsioSetInputMonitor
//                            ASIOFuture calls. Return 1 if yes.
//   kAsioSupportsInputGain: the driver asks the host if it supports input gain setting
//                           via the kAsioSetInputGain ASIOFuture call. Return 1 if yes.
//   kAsioSupportsInputMeter: the driver asks the host if it supports input metering
//                            via the kAsioGetInputMeter ASIOFuture call. Return 1 if yes.
//   kAsioSupportsOutputGain: the driver asks the host if it supports output gain setting
//                            via the kAsioSetOutputGain ASIOFuture call. Return 1 if yes.
//   kAsioSupportsOutputMeter: the driver asks the host if it supports output metering
//                             via the kAsioGetOutputMeter ASIOFuture call. Return 1 if yes.
//   kAsioOverload: the driver detected an overload. Typically, this is caused by the
//                  host application taking too long to process the buffers. The host
//                  should inform the user about the overload.

//-------------------------------------------------------------------------------------------
// ASIOTime* bufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);
// a replacement for bufferSwitch(). When the driver calls this, it will pass the
// address of a ASIOTime structure which contains the timestamp of the audio buffer
// and other information. This callback must be implemented if the host returns 1
// for the kAsioSupportsTimeInfo selector.
// See the ASIOTime definition below.
//-------------------------------------------------------------------------------------------
*/

enum ASIOSelectors
{
	kAsioSelectorSupported = 1,	// selector in <value>, returns 1 if supported
	kAsioEngineVersion,			// returns engine (host) version in <value>
	kAsioResetRequest,			// request driver reset. if accepted, return 1
	kAsioBufferSizeChangeRequest,	// request buffer size change. required size in <value>. if accepted, return 1
	kAsioResyncRequest,			// request driver resync. if accepted, return 1
	kAsioLatenciesChanged,		// informs host that latencies have changed. The host should call ASIOGetLatencies() to update latency.
	kAsioSupportsTimeInfo,		// host supports time info (bufferSwitchTimeInfo) return 1
	kAsioSupportsTimeCode,		// host supports time code reading/writing (ASIOFuture calls) return 1
	kAsioMMCCommand,				// distributor MMC command operation. see description in ASIOCallbacks struct
	kAsioSupportsInputMonitor,	// kAsioSupportsXXX return 1 if host supports this
	kAsioSupportsInputGain,		// "
	kAsioSupportsInputMeter,		// "
	kAsioSupportsOutputGain,		// "
	kAsioSupportsOutputMeter,		// "
	kAsioOverload					// driver detected an overload
};

//-------------------------------------------------------------------------------------------
// ASIO function prototypes
//-------------------------------------------------------------------------------------------
#if MAC
	#if __MACH__
		#ifdef __cplusplus
			extern "C" {
		#endif
		ASIOError ASIOInit(ASIODriverInfo *sysRef); // system reference is used to specify the device
		ASIOError ASIOExit(void);
		ASIOError ASIOStart(void);
		ASIOError ASIOStop(void);
		ASIOError ASIOGetChannels(long *numInputChannels, long *numOutputChannels);
		ASIOError ASIOGetLatencies(long *inputLatency, long *outputLatency);
		ASIOError ASIOGetBufferSize(long *minSize, long *maxSize, long *preferredSize, long *granularity);
		ASIOError ASIOCanSampleRate(ASIOSampleRate sampleRate);
		ASIOError ASIOGetSampleRate(ASIOSampleRate *sampleRate);
		ASIOError ASIOSetSampleRate(ASIOSampleRate sampleRate);
		ASIOError ASIOGetClockSources(ASIOClockSource *clocks, long *numSources);
		ASIOError ASIOSetClockSource(long index);
		ASIOError ASIOGetSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp);
		ASIOError ASIOGetChannelInfo(ASIOChannelInfo *info);
		ASIOError ASIOCreateBuffers(ASIOBufferInfo *bufferInfos, long numChannels, long bufferSize, ASIOCallbacks *callbacks);
		ASIOError ASIODisposeBuffers(void);
		ASIOError ASIOControlPanel(void);
		ASIOError ASIOFuture(long selector, void *opt); 	// TBD, see selectors
		ASIOError ASIOOutputReady(void);
		#ifdef __cplusplus
			}
		#endif
	#else	// old Carbon/Classic interface
		pascal ASIOError ASIOInit(ASIODriverInfo *sysRef);	// system reference is used to specify the device
		pascal ASIOError ASIOExit(void);
		pascal ASIOError ASIOStart(void);
		pascal ASIOError ASIOStop(void);
		pascal ASIOError ASIOGetChannels(long *numInputChannels, long *numOutputChannels);
		pascal ASIOError ASIOGetLatencies(long *inputLatency, long *outputLatency);
		pascal ASIOError ASIOGetBufferSize(long *minSize, long *maxSize, long *preferredSize, long *granularity);
		pascal ASIOError ASIOCanSampleRate(ASIOSampleRate sampleRate);
		pascal ASIOError ASIOGetSampleRate(ASIOSampleRate *sampleRate);
		pascal ASIOError ASIOSetSampleRate(ASIOSampleRate sampleRate);
		pascal ASIOError ASIOGetClockSources(ASIOClockSource *clocks, long *numSources);
		pascal ASIOError ASIOSetClockSource(long index);
		pascal ASIOError ASIOGetSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp);
		pascal ASIOError ASIOGetChannelInfo(ASIOChannelInfo *info);
		pascal ASIOError ASIOCreateBuffers(ASIOBufferInfo *bufferInfos, long numChannels, long bufferSize, ASIOCallbacks *callbacks);
		pascal ASIOError ASIODisposeBuffers(void);
		pascal ASIOError ASIOControlPanel(void);
		pascal ASIOError ASIOFuture(long selector, void *opt); 	// TBD, see selectors
		pascal ASIOError ASIOOutputReady(void);
	#endif
#elif WINDOWS
// note: These functions have __stdcall calling convention!
//       If you implement an ASIO driver, use the C or C++ interface!

	ASIOError __stdcall ASIOInit(ASIODriverInfo *sysRef);
	ASIOError __stdcall ASIOExit(void);
	ASIOError __stdcall ASIOStart(void);
	ASIOError __stdcall ASIOStop(void);
	ASIOError __stdcall ASIOGetChannels(long *numInputChannels, long *numOutputChannels);
	ASIOError __stdcall ASIOGetLatencies(long *inputLatency, long *outputLatency);
	ASIOError __stdcall ASIOGetBufferSize(long *minSize, long *maxSize, long *preferredSize, long *granularity);
	ASIOError __stdcall ASIOCanSampleRate(ASIOSampleRate sampleRate);
	ASIOError __stdcall ASIOGetSampleRate(ASIOSampleRate *sampleRate);
	ASIOError __stdcall ASIOSetSampleRate(ASIOSampleRate sampleRate);
	ASIOError __stdcall ASIOGetClockSources(ASIOClockSource *clocks, long *numSources);
	ASIOError __stdcall ASIOSetClockSource(long index);
	ASIOError __stdcall ASIOGetSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp);
	ASIOError __stdcall ASIOGetChannelInfo(ASIOChannelInfo *info);
	ASIOError __stdcall ASIOCreateBuffers(ASIOBufferInfo *bufferInfos, long numChannels, long bufferSize, ASIOCallbacks *callbacks);
	ASIOError __stdcall ASIODisposeBuffers(void);
	ASIOError __stdcall ASIOControlPanel(void);
	ASIOError __stdcall ASIOFuture(long selector, void *opt);
	ASIOError __stdcall ASIOOutputReady(void);
#endif

// restore alignment rules
#if defined(_MSC_VER) && !defined(__MWERKS__)
#pragma pack(pop)
#elif PRAGMA_ALIGN_SUPPORTED
#pragma options align = reset
#endif

#endif
