#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "15";
	static const char MONTH[] = "01";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.01";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 2;
	static const long MINOR = 3;
	static const long BUILD = 2578;
	static const long REVISION = 7835;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 3719;
	#define RC_FILEVERSION 2,3,2578,7835
	#define RC_FILEVERSION_STRING "2, 3, 2578, 7835\0"
	static const char FULLVERSION_STRING[] = "2.3.2578.7835";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 78;
	

}
#endif //VERSION_H
