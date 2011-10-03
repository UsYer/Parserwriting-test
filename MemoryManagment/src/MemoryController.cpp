
#include <map>
#include <list>
//#include <boost/variant.hpp>
#include <boost/circular_buffer.hpp>
#include "../../include/Internal/Table.hpp"
#include "../include/MemoryController.hpp"
#include "../include/Reference.hpp"
#include "../include/Exceptions.hpp"
#if (MEMORY_MANAGMENT_PROJECT || DEBUG)
#include <iostream>
#endif
using namespace Internal;
//Internal methods, not visible to References
class MemoryController::InternalImpl
{
    public:
    friend class MemoryController;
    InternalImpl():
        UsedIDs(boost::circular_buffer_space_optimized<ID>::capacity_type(1600,400)),
        LastIdNumber(0)
    {}
    private:
    void Delete(ID Id) //Delete() is located here becuase the Reference friends shall not be able to call it
    {
        Memory.erase(AddressMap[Id]);
        AddressMap.erase(Id);
        RefCounter.erase(Id);
        UsedIDs.push_back(Id);
#ifdef DEBUG
        std::cout << "Memorycontroller: Deleting Item with Id " << Id << "\n";
#endif
    }

    typedef std::list<Value>::iterator Address;

    std::list<Value> Memory;
    std::map< ID, Address > AddressMap;//Provides the addresses to the corresponding IDs
    std::map< ID,unsigned int /*RefCounter*/> RefCounter;
    boost::circular_buffer_space_optimized<ID> UsedIDs;
    ID LastIdNumber;
};
MemoryController::MemoryController():
    Internal(new InternalImpl)
{
}
MemoryController::~MemoryController()
{
    delete Internal;
}

CountedReference MemoryController::Save(const Value& Val)
{
    ID Id;
    if( !Internal->UsedIDs.empty() )
    {
        Id = Internal->UsedIDs.back();
        Internal->UsedIDs.pop_back();
    }
    else
    {
        Id = Internal->LastIdNumber++;
    }
    Internal->Memory.push_back(Val);
    auto it = Internal->Memory.end()--;
// NOTE (Marius#1#): Is there a better way to get the iterator to the newly inserted element?
//    std::advance(it, Internal->Memory.size()-1);
    Internal->AddressMap[Id] = it;
    Internal->RefCounter[Id] = 1;
#ifdef DEBUG
    std::cout << "Memorycontroller: Creating Item with Id " << Id << "\n";
#endif
    return CountedReference(this, Id);
}

void MemoryController::IncRef( const CountedReference& Ref )
{
    Internal->RefCounter[Ref.m_ID] += 1;
#ifdef DEBUG
    //std::cout << "Memorycontroller: Increasing refcount of Item " << Ref.m_ID << "; is now " << Internal->RefCounter[Ref.m_ID] << "\n";
#endif
}

void MemoryController::DecRef( const CountedReference& Ref )
{
    auto it = Internal->RefCounter.find(Ref.m_ID);
    if( it != Internal->RefCounter.end() )
    {
#ifdef DEBUG
        //std::cout << "Memorycontroller: Decreasing refcount of Item " << Ref.m_ID << "; is now " << Internal->RefCounter[Ref.m_ID] - 1 << "\n";
#endif
        if( (it->second -= 1) <= 0 )
        {
            Internal->Delete(Ref.m_ID);
        }
    }
}

unsigned int MemoryController::GetRefCount(const CountedReference& Ref) const
{
    auto it = Internal->RefCounter.find(Ref.m_ID);
    if( it != Internal->RefCounter.end() )
    {
        return it->second;
    }
    return 0;
}
#ifdef MEMORY_MANAGMENT_PROJECT
void MemoryController::Print() const
{
    std::cout << "\tID\tAddress\tRefCout\tValue\n";
    for(auto it = Internal->AddressMap.begin(); it != Internal->AddressMap.end(); ++it)
    {
        std::cout << "\t" << it->first << "\t" << &(*(it->second)) << "\t";
        auto i = Internal->RefCounter.find(it->first);
//        std::cout << i->second << "\t" << *(it->second) << "\n";
    }
    std::cout << std::endl;
}
#endif
MemoryController::Value& MemoryController::GetValue(const Reference& Ref) const
{
    //No check for a null reference necessary here, because that happens in the reference class
    auto it = Internal->AddressMap.find(Ref.m_ID);
    return *(it->second);
}

unsigned int MemoryController::Size() const
{
    return Internal->Memory.size();
}
