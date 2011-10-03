#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "03";
	static const char MONTH[] = "10";
	static const char YEAR[] = "2011";
	static const char UBUNTU_VERSION_STYLE[] = "11.10";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 2;
	static const long MINOR = 3;
	static const long BUILD = 2571;
	static const long REVISION = 7816;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 3698;
	#define RC_FILEVERSION 2,3,2571,7816
	#define RC_FILEVERSION_STRING "2, 3, 2571, 7816\0"
	static const char FULLVERSION_STRING[] = "2.3.2571.7816";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 71;
	

}
#endif //VERSION_H
