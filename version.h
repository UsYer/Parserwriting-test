#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "15";
	static const char MONTH[] = "09";
	static const char YEAR[] = "2011";
	static const char UBUNTU_VERSION_STYLE[] = "11.09";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 2;
	static const long MINOR = 3;
	static const long BUILD = 2562;
	static const long REVISION = 7791;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 3671;
	#define RC_FILEVERSION 2,3,2562,7791
	#define RC_FILEVERSION_STRING "2, 3, 2562, 7791\0"
	static const char FULLVERSION_STRING[] = "2.3.2562.7791";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 62;
	

}
#endif //VERSION_H
