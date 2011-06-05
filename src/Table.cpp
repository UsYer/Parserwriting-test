#include "../MemoryManagment/include/MemoryController.hpp"
#include "../MemoryManagment/include/Reference.hpp"
//#include "../include/IFunction.hpp"
#include "../include/Internal/Table.hpp"
#include <vector>
#include <map>
//#include <sstream>
using Internal::Member;
using namespace Internal::Types;
struct Table::Implementation
{
    const Member Null;
    Implementation():
        Null(NullReference())
    {
    }
    ~Implementation()
    {}
    Member& GetMemberViaIdentifer(const std::string& Identifer)
    {
        auto it = m_AssocData.find(Identifer);
        if(it == m_AssocData.end())
        {
            CountedReference Null((NullReference()));
            //m_AssocData[Identifer] = Null;
            //return Null;
            return m_AssocData[Identifer] = Null;
        }
        else
            return m_AssocData[Identifer];
    }
    Member& GetMemberViaIndex(unsigned int Index)
    {
        auto it = m_IndexData.find(Index);
        if(it == m_IndexData.end())
        {
            CountedReference Null((NullReference()));
            //m_IndexData[Index] = Null;
            //return Null;
            return m_IndexData[Index] = Null;
        }
        else
            return m_IndexData[Index];
    }
    std::map<std::string,Member> m_AssocData;
    std::map<unsigned,Member> m_IndexData;
};

Table::Table():
    Impl(new Implementation)
{
}
Table::Table(const Table& Other)
{
    if( &Other != this )
    {
        Impl = new Implementation;
        Impl->m_AssocData = Other.Impl->m_AssocData;
        Impl->m_IndexData = Other.Impl->m_IndexData;
    }
}
Table::Table( Table&& Other )
{
    Impl = Other.Impl;
    Other.Impl = 0;
}
Table& Table::operator=(Table Other)
{
    Impl = Other.Impl;
    std::swap(Impl->m_AssocData,Other.Impl->m_AssocData);
    std::swap(Impl->m_IndexData,Other.Impl->m_IndexData);
    return *this;
}
Table::~Table()
{
    delete Impl;
}

Member& Table::operator[](const std::string& Identifier)
{
    return Impl->GetMemberViaIdentifer(Identifier);
}
Member& Table::operator[](unsigned int Index)
{
    return Impl->GetMemberViaIndex(Index);
}
const Member& Table::operator[](const std::string& Identifier) const
{
    return Impl->GetMemberViaIdentifer(Identifier);
}
const Member& Table::operator[](unsigned int Index) const
{
    return Impl->GetMemberViaIndex(Index);
}
Member& Table::Add(const std::pair<std::string,Member>& KeyValue)
{
    return Impl->m_AssocData[KeyValue.first] = KeyValue.second;
}
unsigned int Table::Add(const Member& Value)
{
    Impl->m_IndexData.insert(std::make_pair(Impl->m_IndexData.size(),Value));
    return Impl->m_IndexData.size()-1;
}
Member& Table::Add(const std::string& Key, const Member& Value)
{
    return Impl->m_AssocData[Key] = Value;
}
/*inline*/ Table::KeyIterator Table::KeyBegin()
{
    return Impl->m_AssocData.begin();
}
/*inline*/ Table::KeyIterator Table::KeyEnd()
{
    return Impl->m_AssocData.end();
}
/*inline*/ Table::IndexIterator Table::IndexBegin()
{
    return Impl->m_IndexData.begin();
}
/*inline*/ Table::IndexIterator Table::IndexEnd()
{
    return Impl->m_IndexData.end();
}
/*inline*/ Table::ConstKeyIterator Table::KeyBegin() const
{
    return Impl->m_AssocData.cbegin();
}
/*inline*/ Table::ConstKeyIterator Table::KeyEnd()const
{
    return Impl->m_AssocData.cend();
}
/*inline*/ Table::ConstIndexIterator Table::IndexBegin()const
{
    return Impl->m_IndexData.cbegin();
}
/*inline*/ Table::ConstIndexIterator Table::IndexEnd()const
{
    return Impl->m_IndexData.cend();
}
unsigned Table::KeySize() const
{
    return Impl->m_AssocData.size();
}
unsigned Table::IndexSize() const
{
    return Impl->m_IndexData.size();
}
bool Table::Contains(const std::string& Key) const
{
    auto it = Impl->m_AssocData.find(Key);
    return it != Impl->m_AssocData.end();
}
bool Table::Contains(unsigned int Index) const
{
    auto it = Impl->m_IndexData.find(Index);
    return it != Impl->m_IndexData.end();
}
Table::KeyIterator Table::Find(const std::string& Key) const
{
    return Impl->m_AssocData.find(Key);
}
Table::IndexIterator Table::Find(unsigned int Index) const
{
    return Impl->m_IndexData.find(Index);
}
void Table::Clear()
{
    Impl->m_AssocData.clear();
    Impl->m_IndexData.clear();
}
void Table::ClearKeyValues()
{
    Impl->m_AssocData.clear();
}
void Table::ClearIndexValues()
{
    Impl->m_IndexData.clear();
}
