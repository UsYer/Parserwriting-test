#include <fstream>
#include <ctime>
#include <sstream>
#include "../include/MariusParser.hpp"


int main(int argc, const char* argv[])
{
    BenchData benchData;
    MariusParser MP;
    std::stringstream s;
    for(unsigned i = 0; i < 100; ++i)
    {
        MP.Evaluate("pi = Math.Pi");
        MP.Evaluate("e = Math.E");
        MP.Evaluate("sqrt = Math.Sqrt");
        MP.Evaluate("(((-9))-e/(((((((pi-(((-7)+(-3)/4/e))))/(((-5))-2)-((pi+(-0))*(sqrt((e+e))*(-8))*"
                    "(((-pi)+(-pi)-(-9)*(6*5))/(-e)-e))/2)/((((sqrt(2/(-e)+6)-(4-2))+((5/(-2))/(1*(-pi)"
                    "+3))/8)*pi*((pi/((-2)/(-6)*1*(-1))*(-6)+(-e)))))/((e+(-2)+(-e)*((((-3)*9+(-e)))+"
                    "(-9)))))))-((((e-7+(((5/pi-(3/1+pi)))))/e)/(-5))/(sqrt((((((1+(-7))))+((((-e)*(-e)))"
                    "-8))*(-5)/((-e)))*(-6)-((((((-2)-(-9)-(-e)-1)/3))))/(sqrt((8+(e-((-6))+(9*(-9))))"
                    "*(((3+2-8))*(7+6+(-5))+((0/(-e)*(-pi))+7)))+(((((-e)/e/e)+((-6)*5)*e+(3+(-5)/pi))))"
                    "+pi))/sqrt((((9))+((((pi))-8+2))+pi))/e*4)*((-5)/(((-pi))*(sqrt(e)))))-(((((((-e)*(e)"
                    "-pi))/4+(pi)*(-9)))))))+(-pi)", &benchData);
        s << benchData.TicksTokenize << "\t" << benchData.TicksParse << "\t" << benchData.TicksEval << "\t" << benchData.TicksSum << "\n";
    }
    const char* fileName = "result.csv";
    std::ofstream file(fileName);
    if( file )
    {
        std::time_t timeInSeconds = std::time(0);
        auto timeInfo = std::localtime(&timeInSeconds);
        file << MariusParser::VersionData::FullversionString << "\t" << std::asctime(timeInfo)
             << "Tokenize\tParse\tEval\tAll\n";
        file << s.str();
    }
    else
        std::cerr << "error opening " << fileName << "\n";
}
