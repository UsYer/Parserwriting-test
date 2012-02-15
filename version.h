#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "07";
	static const char MONTH[] = "02";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.02";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 2;
	static const long MINOR = 4;
	static const long BUILD = 2600;
	static const long REVISION = 7898;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 3761;
	#define RC_FILEVERSION 2,4,2600,7898
	#define RC_FILEVERSION_STRING "2, 4, 2600, 7898\0"
	static const char FULLVERSION_STRING[] = "2.4.2600.7898";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 0;
	

}
#endif //VERSION_H
