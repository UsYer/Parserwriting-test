#include "../MemoryManagment/include/MemoryController.hpp"
#include "../MemoryManagment/include/Reference.hpp"
#include "../include/Internal/Table.hpp"
#include <vector>
using Internal::Member;
using namespace Internal::Types;
using namespace boost;

struct Table::Implementation
{
    const Member Null;
    Implementation():
        Null(NullReference())
    {
    }
    ~Implementation()
    {}
    Member& GetMemberViaIdentifer(const std::string& Identifier)
    {
        auto& view = get<key>(m_AssocData);
        auto it = view.find(Identifier);
//        auto it = m_AssocData.find(Identifier);
//        if(it == m_AssocData.end())
        if(it == view.end())
        {
            CountedReference Null = (NullReference());
//            return m_AssocData[Identifer] = Null;
            auto result = m_AssocData.insert(Item(Identifier,Null));
            return result.first->second;//insert returns a pair<iterator,bool> but we disregard the bool type since we already know that there is no conflicting item already present
        }
        else
            return it->second;
//            return m_AssocData[Identifier];
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
//    std::unordered_map<std::string,Member> m_AssocData; // FIXME (Marius#9#): unordered_map doesn't keep them in the order the members were added. Leads to function args that are applied in the wrong order
    Table::AssocData m_AssocData;
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
    Other.Impl = nullptr;
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
//    return Impl->m_AssocData[KeyValue.first] = KeyValue.second;
    return Add(KeyValue.first,KeyValue.second);
}
unsigned int Table::Add(const Member& Value)
{
    Impl->m_IndexData.insert(std::make_pair(Impl->m_IndexData.size(),Value));
    return Impl->m_IndexData.size()-1;
}
Member& Table::Add(const std::string& Key, const Member& Value)
{
//    return Impl->m_AssocData[Key] = Value;
    auto& view = get<key>(Impl->m_AssocData);
    auto it = view.find(Key);
    if(it == view.end())
    {
        auto result = Impl->m_AssocData.insert(Item(Key,Value));
        return result.first->second;
    }
    else
    {
        return (it->second = Value);
        /*
        view.modify(it,[&Value](Item& item){item.second = Value;});
        */
    }
}
Table::KeyIterator Table::KeyBegin()
{
    return get<key_seq>(Impl->m_AssocData).begin();
}
Table::KeyIterator Table::KeyEnd()
{
    return get<key_seq>(Impl->m_AssocData).end();
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
    return get<key_seq>(Impl->m_AssocData).cbegin();
}
/*inline*/ Table::ConstKeyIterator Table::KeyEnd()const
{
    return get<key_seq>(Impl->m_AssocData).cend();
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
    auto& hashedView = get<key>(Impl->m_AssocData);
    return hashedView.find(Key) != hashedView.end();
}
bool Table::Contains(unsigned int Index) const
{
    auto it = Impl->m_IndexData.find(Index);
    return it != Impl->m_IndexData.end();
}
Table::KeyIterator Table::Find(const std::string& Key) const
{
    auto KeyIt = get<key>(Impl->m_AssocData).find(Key);
    return Impl->m_AssocData.project<key_seq>(KeyIt);
}
Table::IndexIterator Table::Find(unsigned int Index) const
{
    return Impl->m_IndexData.find(Index);
}
void Table::ForEachKey(const std::function<void(Item&)>& ModifyFunc)
{
    auto& sequencedView = get<key_seq>(Impl->m_AssocData);
    for( auto it = sequencedView.begin(), endIt = sequencedView.end(); it != endIt; std::advance(it,1) )
    {
        sequencedView.modify(it,ModifyFunc);
    }
}
void Table::ForSomeKeys(unsigned HowMany, const std::function<void(Item&)>& ModifyFunc)
{
    auto& sequencedView = get<key_seq>(Impl->m_AssocData);
    auto it = sequencedView.begin(), endIt = sequencedView.end();
    for( unsigned i = 0; i < HowMany && it != endIt; std::advance(it,1), i++ )
    {
        sequencedView.modify(it,ModifyFunc);
    }
}
bool Table::ModifyByKey(const std::string& Key, const std::function<void(Item&)>& ModifyFunc)
{
    return ModifyByKey(Find(Key), ModifyFunc);
}
bool Table::ModifyByKey(ConstKeyIterator KeyIt, const std::function<void(Item&)>& ModifyFunc)
{
    if( KeyIt != KeyEnd() )
    {
        return get<key_seq>(Impl->m_AssocData).modify(KeyIt, ModifyFunc);
    }
    return false;
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
