#ifndef __asiotypes__
#define __asiotypes__

// Platform definitions
#if SGI 
	#undef BEOS 
	#undef MAC 
	#undef WINDOWS
	//
	#define ASIO_BIG_ENDIAN 1
	#define ASIO_CPU_MIPS 1
#elif defined(_WIN32) || defined(_WIN64)
	#undef BEOS 
	#undef MAC 
	#undef SGI
	#define WINDOWS 1
	#define ASIO_LITTLE_ENDIAN 1
	#define ASIO_CPU_X86 1
#elif BEOS
	#undef MAC 
	#undef SGI
	#undef WINDOWS
	#define ASIO_LITTLE_ENDIAN 1
	#define ASIO_CPU_X86 1
	//
#else
	#define MAC 1
	#undef BEOS 
	#undef WINDOWS
	#undef SGI
	#define ASIO_BIG_ENDIAN 1
	#define ASIO_CPU_PPC 1
#endif

// always
#define NATIVE_INT64 0
#define IEEE754_64FLOAT 1

//- - - - - - - - - - - - - - - - - - - - - - - - -
// Type definitions
//- - - - - - - - - - - - - - - - - - - - - - - - -

// number of samples data type is 64 bit integer
#if NATIVE_INT64
	typedef long long int ASIOSamples;
#else
	typedef struct ASIOSamples {
		unsigned long hi;
		unsigned long lo;
	} ASIOSamples;
#endif

// Timestamp data type is 64 bit integer,
// Time format is Nanoseconds.
#if NATIVE_INT64
	typedef long long int ASIOTimeStamp;
#else
	typedef struct ASIOTimeStamp {
		unsigned long hi;
		unsigned long lo;
	} ASIOTimeStamp;
#endif

// Samplerates are expressed in IEEE 754 64 bit double float,
// native format as host computer
#if IEEE754_64FLOAT
	typedef double ASIOSampleRate;
#else
	typedef struct ASIOSampleRate {
		char ieee[8];
	} ASIOSampleRate;
#endif

// Boolean values are expressed as long
typedef long ASIOBool;
enum {
	ASIOFalse = 0,
	ASIOTrue = 1
};

// Sample Types are expressed as long
typedef long ASIOSampleType;
enum {
	ASIOSTInt16MSB   = 0,
	ASIOSTInt24MSB   = 1,		// used for 20 bits as well
	ASIOSTInt32MSB   = 2,
	ASIOSTFloat32MSB = 3,		// IEEE 754 32 bit float
	ASIOSTFloat64MSB = 4,		// IEEE 754 64 bit double float

	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can be more easily used with these
	ASIOSTInt32MSB16 = 8,		// 32 bit data with 16 bit alignment
	ASIOSTInt32MSB18 = 9,		// 32 bit data with 18 bit alignment
	ASIOSTInt32MSB20 = 10,		// 32 bit data with 20 bit alignment
	ASIOSTInt32MSB24 = 11,		// 32 bit data with 24 bit alignment
	
	ASIOSTInt16LSB   = 16,
	ASIOSTInt24LSB   = 17,		// used for 20 bits as well
	ASIOSTInt32LSB   = 18,
	ASIOSTFloat32LSB = 19,		// IEEE 754 32 bit float, as found on Intel x86 architecture
	ASIOSTFloat64LSB = 20, 		// IEEE 754 64 bit double float, as found on Intel x86 architecture

	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can more easily used with these
	ASIOSTInt32LSB16 = 24,		// 32 bit data with 16 bit alignment
	ASIOSTInt32LSB18 = 25,		// 32 bit data with 18 bit alignment
	ASIOSTInt32LSB20 = 26,		// 32 bit data with 20 bit alignment
	ASIOSTInt32LSB24 = 27,		// 32 bit data with 24 bit alignment

	ASIOSTDSDInt8LSB1   = 32,		// DSD 1 bit data, 8 samples per byte. First sample in Least significant bit.
	ASIOSTDSDInt8MSB1   = 33,		// DSD 1 bit data, 8 samples per byte. First sample in Most significant bit.
	ASIOSTDSDInt8NER8   = 40,		// DSD 8 bit data, 1 sample per byte. No Endianness required.
};

// Error codes
typedef long ASIOError;
enum {
	ASE_OK = 0,				// This value will be returned whenever the call succeeded
	ASE_SUCCESS = 0x3f4847a0,	// unique success return value for ASIOFuture calls
	ASE_NotPresent = -1000,	// hardware input or output is not present or available
	ASE_HWMalfunction,		// hardware is malfunctioning (can be returned by any ASIO function)
	ASE_InvalidParameter,	// input parameter invalid
	ASE_InvalidMode,		// hardware is in a bad mode or used in a bad mode
	ASE_SPNotAdvancing,		// hardware is not running when sample position is inquired
	ASE_NoClock,			// sample clock or rate cannot be determined or is not present
	ASE_NoMemory			// not enough memory for completing the request
};

//-------------------------------------------------------------------------------------------
// Time Info
//-------------------------------------------------------------------------------------------
typedef struct ASIOTimeInfo
{
	double          speed;                  // absolute speed (1. = nominal)
	ASIOTimeStamp   systemTime;             // system time related to samplePosition, in nanoseconds
	                                        // on mac, must be derived from Microseconds() (not UpTime()!)
	                                        // on windows, must be derived from timeGetTime()
	ASIOSamples     samplePosition;
	ASIOSampleRate  sampleRate;             // current rate
	unsigned long flags;                    // (see below)
	char reserved[12];
} ASIOTimeInfo;

typedef enum ASIOTimeInfoFlags
{
	kSystemTimeValid        = 1,            // must always be valid
	kSamplePositionValid    = 1 << 1,       // must always be valid
	kSampleRateValid        = 1 << 2,
	kSpeedValid             = 1 << 3,
	
	kSampleRateChanged      = 1 << 4,
	kClockSourceChanged     = 1 << 5
} ASIOTimeInfoFlags;

//-------------------------------------------------------------------------------------------
// Time Code
//-------------------------------------------------------------------------------------------
typedef struct ASIOTimeCode
{
	double          speed;                  // speed relation (fraction of nominal speed)
	                                        // optional; set to 0. or 1. if not supported
	ASIOSamples     timeCodeSamples;        // time in samples
	unsigned long   flags;                  // some information flags (see below)
	char future[64];
} ASIOTimeCode;

typedef enum ASIOTimeCodeFlags
{
	kTcValid                = 1,            // time code valid
	kTcRunning              = 1 << 1,       // time code is running
	kTcReverse              = 1 << 2,       // time code is running backwards
	kTcOnspeed              = 1 << 3,       // time code is running at nominal speed
	kTcStill                = 1 << 4,       // time code is still
	kTcSpeedValid           = 1 << 8        // speed is valid
} ASIOTimeCodeFlags;

#endif	// __asiotypes__
