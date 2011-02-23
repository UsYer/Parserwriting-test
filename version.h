#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "23";
	static const char MONTH[] = "02";
	static const char YEAR[] = "2011";
	static const char UBUNTU_VERSION_STYLE[] = "11.02";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 2;
	static const long MINOR = 0;
	static const long BUILD = 2244;
	static const long REVISION = 6779;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 3175;
	#define RC_FILEVERSION 2,0,2244,6779
	#define RC_FILEVERSION_STRING "2, 0, 2244, 6779\0"
	static const char FULLVERSION_STRING[] = "2.0.2244.6779";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 44;
	

}
#endif //VERSION_H
