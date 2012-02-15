#ifndef TABLE_H
#define TABLE_H
#include <map>
#include <unordered_map>
#include <string>
#include <memory>
#include "Tokens.hpp"
namespace Internal
{
namespace Types
{
//class CountedReference;
class Table
{
    public:
        typedef std::unordered_map<std::string,Member>::iterator KeyIterator;
        typedef std::map<unsigned,Member>::iterator IndexIterator;
        typedef std::unordered_map<std::string,Member>::const_iterator ConstKeyIterator;
        typedef std::map<unsigned,Member>::const_iterator ConstIndexIterator;
        /** Default constructor */
        Table();
        Table(const Table& Other);
        Table( Table&& Other );
        /** Default destructor */
        ~Table();
        Table& operator=(Table Other);
        Member& operator[](const std::string& Identifier);
        Member& operator[](unsigned int Index);
        const Member& operator[](const std::string& Identifier) const;
        const Member& operator[](unsigned int Index) const;
        /**
        Adds an Key-Value Pair to the table

        This method adds a new Key-Value pair to the table
        @return The zero-based index of the newly inserted value
        */
        Member& Add(const std::pair<std::string,Member>& KeyValue);
        Member& Add(const std::string& Key, const Member& Value);
        unsigned int Add(const Member& Value);
        KeyIterator KeyBegin();
        KeyIterator KeyEnd();
        IndexIterator IndexBegin();
        IndexIterator IndexEnd();
        ConstKeyIterator KeyBegin() const;
        ConstKeyIterator KeyEnd() const;
        ConstIndexIterator IndexBegin() const;
        ConstIndexIterator IndexEnd() const;
        unsigned KeySize() const;
        unsigned IndexSize() const;
        bool Contains(const std::string& Key) const;
        bool Contains(unsigned int Index) const;
        KeyIterator Find(const std::string& Key) const;
        IndexIterator Find(unsigned int Index) const;
        void Clear();
        void ClearKeyValues();
        void ClearIndexValues();
    protected:
    private:
        struct Implementation; //Pimpl-
        Implementation* Impl;  //Idiom
};
}
}
#endif // TABLE_H
