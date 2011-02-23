#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "24";
	static const char MONTH[] = "02";
	static const char YEAR[] = "2011";
	static const char UBUNTU_VERSION_STYLE[] = "11.02";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 2;
	static const long MINOR = 0;
	static const long BUILD = 2250;
	static const long REVISION = 6793;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 3183;
	#define RC_FILEVERSION 2,0,2250,6793
	#define RC_FILEVERSION_STRING "2, 0, 2250, 6793\0"
	static const char FULLVERSION_STRING[] = "2.0.2250.6793";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 50;
	

}
#endif //VERSION_H
