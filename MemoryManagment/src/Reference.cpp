#include "../include/MemoryController.hpp"
#include "../include/Reference.hpp"
#include "../../include/Exceptions.hpp"
#include "../../include/Internal/Table.hpp"
using namespace Internal;
Reference::Reference( const Reference& Other ):
    m_MC(Other.m_MC),
    m_ID(Other.m_ID)
{
}

Reference& Reference::operator=( Reference Other )
{
    if( this != &Other )
    {
        Other.Swap(*this);
    }
    return *this;
}

MemoryController::Value& Reference::operator*() const
{
    if( !m_MC )
        throw Exceptions::NullReferenceException("Dereferencing a null reference");

    return m_MC->GetValue(*this);
}

void Reference::Swap(Reference& Other)
{
    std::swap(m_MC, Other.m_MC);
    std::swap(m_ID, Other.m_ID);
}

Reference::~Reference()
{
}

CountedReference::CountedReference( const Reference& Other ):
    //Reference( (Other.IsNull() ? throw NullReferenceException("Can't create a counted reference from a null reference") : Other) )
    Reference(Other)
{
    //Releasing the actual value here is not neccesary, because this is the cpy-ctor and the Reference can't point to anything at this moment
    //m_MC->IncRef(*this);
    IncRef();
}

//This explicit copy-ctor for CountedReferences is needed, because otherwise Reference::Reference( const Reference& ) would be called
CountedReference::CountedReference( const CountedReference& Other ):
    Reference(Other.m_MC, Other.m_ID)
{
    //Releasing the actual value here is not neccesary, because this is the cpy-ctor and the Reference can't point to anything at this moment
    //m_MC->IncRef(*this);
    IncRef();
}

CountedReference& CountedReference::operator=( Reference Other )
{
    //No check for self assignment necessary, because a plain Reference is never the same as a CountedReference
    //if( Other.IsNull() )
        //throw NullReferenceException("Can't assign a null reference to a counted reference");

    /*Other.Swap(*this);
    m_MC->DecRef(Other); //DecRef the value this reference has pointed to, before the assignment happened
    m_MC->IncRef(*this);*/
    DecRef();
    Other.Swap(*this);
    IncRef();
    return *this;
}

CountedReference& CountedReference::operator=( CountedReference Other )
{
    if( this != &Other )
    {
        Other.Swap(*this);
        //Don't be tempted to manually DecRef() the Other CountedReference which just got switched with this one
        //because it will autoamtically be DecRefed after going out of the scope of this function.
        //If you still do it, it'll be DecRefed one time too often and be deleted too early, leading to a crash

        //Also, don't IncRef it here manually.
    }
    return *this;
}
// TODO (Marius#8#): Add R-Value aware assign-op
/*inline*/ void CountedReference::DecRef()const
{
    if( m_MC )
        m_MC->DecRef(*this);
}

/*inline*/ void CountedReference::IncRef()const
{
    if( m_MC )
        m_MC->IncRef(*this);
}
CountedReference::~CountedReference()
{
    //m_MC->DecRef(*this);
    DecRef();
}
