#ifndef PLATFORM_HPP_INCLUDED
#define PLATFORM_HPP_INCLUDED
#include <iosfwd>
namespace Marius
{
	namespace Platform
	{

		void OpenConsole();
		std::ostream& Out();
		std::istream& In();
	}// ns Platform
}//ns Marius
#endif // PLATFORM_HPP_INCLUDED
