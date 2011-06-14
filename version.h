#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "14";
	static const char MONTH[] = "06";
	static const char YEAR[] = "2011";
	static const char UBUNTU_VERSION_STYLE[] = "11.06";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 2;
	static const long MINOR = 2;
	static const long BUILD = 2483;
	static const long REVISION = 7528;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 3563;
	#define RC_FILEVERSION 2,2,2483,7528
	#define RC_FILEVERSION_STRING "2, 2, 2483, 7528\0"
	static const char FULLVERSION_STRING[] = "2.2.2483.7528";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 83;
	

}
#endif //VERSION_H
