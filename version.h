#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "09";
	static const char MONTH[] = "02";
	static const char YEAR[] = "2014";
	static const char UBUNTU_VERSION_STYLE[] =  "14.02";
	
	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long MAJOR  = 2;
	static const long MINOR  = 4;
	static const long BUILD  = 2667;
	static const long REVISION  = 8101;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 3858;
	#define RC_FILEVERSION 2,4,2667,8101
	#define RC_FILEVERSION_STRING "2, 4, 2667, 8101\0"
	static const char FULLVERSION_STRING [] = "2.4.2667.8101";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 67;
	

}
#endif //VERSION_H
