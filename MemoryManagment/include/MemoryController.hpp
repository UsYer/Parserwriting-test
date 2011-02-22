#ifndef MEMORYCONTROLLER_H
#define MEMORYCONTROLLER_H
#include <map>
#include <list>
//#include <boost/variant.hpp>
#include <boost/circular_buffer.hpp>
namespace Internal
{
class Reference;
class CountedReference;
namespace Types
{
    class Table;
}
class MemoryController
{
    friend class Reference;
    friend class CountedReference;
    //friend class Internal;
public:
    typedef unsigned long ID;
    typedef Types::Table Value;

    MemoryController();
    ~MemoryController();
    MemoryController(const MemoryController&) = delete;
    MemoryController& operator=(const MemoryController&) = delete;

    CountedReference Save(const Value& Val);
    unsigned int GetRefCount(const CountedReference& Ref) const;
    unsigned int Size() const;
#ifdef MEMORY_MANAGMENT_PROJECT
    void Print()const;
#endif
protected:

private:
    void IncRef( const CountedReference& Ref );
    void DecRef( const CountedReference& Ref );
    Value& GetValue( const Reference& Ref ) const;

    class InternalImpl;
    InternalImpl* Internal;
};
}

#endif // MEMORYCONTROLLER_H
