#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "16";
	static const char MONTH[] = "09";
	static const char YEAR[] = "2011";
	static const char UBUNTU_VERSION_STYLE[] = "11.09";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 2;
	static const long MINOR = 3;
	static const long BUILD = 2570;
	static const long REVISION = 7812;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 3693;
	#define RC_FILEVERSION 2,3,2570,7812
	#define RC_FILEVERSION_STRING "2, 3, 2570, 7812\0"
	static const char FULLVERSION_STRING[] = "2.3.2570.7812";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 70;
	

}
#endif //VERSION_H
