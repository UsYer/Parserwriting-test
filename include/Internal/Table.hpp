#ifndef TABLE_H
#define TABLE_H
#include <map>
//#include <unordered_map>
#include <string>
#include <functional>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include "Tokens.hpp"
namespace Internal
{
namespace Types
{
//class CountedReference;
class Table
{
    public:
        struct key_seq{};
        struct key{};

        struct Item
        {
            std::string first;
            mutable Member second; //made mutable so it's changeable via iterators. found at: http://stackoverflow.com/questions/5528517/getting-around-boost-multi-index-containers-constant-elemets
            Item (const std::string& key_v, const Member& data_v):
                 first(key_v),
                 second(data_v)
                 {}
        };
        typedef boost::multi_index::multi_index_container<
        Item,
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_unique<boost::multi_index::tag<key>, boost::multi_index::member<Item,std::string,&Item::first>>,
            boost::multi_index::sequenced<boost::multi_index::tag<key_seq> >
            >
        > AssocData;
        typedef boost::multi_index::index<AssocData,key_seq>::type Map_sequenced_by_key_index_t;
        typedef boost::multi_index::index<AssocData,key_seq>::type::iterator  KeyIterator;
        typedef boost::multi_index::index<AssocData,key_seq>::type::const_iterator  ConstKeyIterator;
        typedef std::map<unsigned,Member>::iterator IndexIterator;
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
        IndexIterator IndexBegin();
        IndexIterator IndexEnd();
        KeyIterator KeyBegin();
        KeyIterator KeyEnd();
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
        void ForEachKey(const std::function<void(Item&)>& ModifyFunc);
        void ForSomeKeys(unsigned HowMany, const std::function<void(Item&)>& ModifyFunc);
        bool ModifyByKey(const std::string& Key, const std::function<void(Item&)>& ModifyFunc);
        bool ModifyByKey(ConstKeyIterator KeyIt, const std::function<void(Item&)>& ModifyFunc);
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
