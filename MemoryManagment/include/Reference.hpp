#ifndef REFERENCE_H
#define REFERENCE_H
namespace Internal
{
class NullReference
{
    public:
        /*NullReference() = default;
        //NullReference(const NullReference&) = delete;
        ~NullReference()
        {}*/
    protected:

    private:

};


class MemoryController;
class CountedReference;
namespace Types
{
    class Table;
}
class Reference
{
    friend class MemoryController;
    friend class CountedReference;
public:
//    Reference() = delete;
    ///Default constructor for the creation of a null reference
    Reference():
        m_MC(0)
    {
    }
    Reference( const Reference& Other );
    Reference( const CountedReference& ) = delete;
    ///Exeption safe Assignmentoperator
    Reference& operator=( Reference Other );//Pass by Value to support copy ellision
    Reference& operator=( const CountedReference& ) = delete; //don't allow implicit creation of a WeakRef through an assignment.
                                                              //Use CountedReference::GetWeakReference() instead
    /*MemoryController::Value*/Types::Table& operator*() const;
    bool IsNull()const
    {
        //No valid pointer pointer to the MemoryController means that this reference points to nothing
        return !m_MC;
    }
    virtual ~Reference();
protected:
    void Swap(Reference& Other);
    MemoryController* m_MC;
    unsigned long m_ID;
    Reference( MemoryController* MC, unsigned long ID ):
        m_MC(MC),
        m_ID(ID)
    {
    }
private:
};

class CountedReference : public Reference
{
    friend class MemoryController;
public:
    CountedReference() = delete;
    CountedReference( const Reference& Other );
    CountedReference( const CountedReference& Other );
    //Nulled CountedReferences have always be created explicitly:
    CountedReference( NullReference ):
        Reference(0,0)
    {}
    CountedReference& operator=( Reference Other );
    CountedReference& operator=( CountedReference Other );
    CountedReference& operator=( NullReference )
    {
        DecRef();
        m_MC = 0;
        return *this;
    }
    virtual ~CountedReference();

    Reference GetWeakReference() const
    {
        return Reference(m_MC, m_ID);
    }
private:
    CountedReference( MemoryController* MC, unsigned long ID ):
        Reference(MC,ID)
    {
    }
    void DecRef()const;
    void IncRef()const;
};
}
#endif // REFERENCE_H
