#include <iostream>
#include "../include/Internal/Table.hpp"
#include "include/MemoryController.hpp"
#include "include/Reference.hpp"
#include <vector>
using namespace Internal;
int main()
{
    MemoryController MC;
    {
    CountedReference Foo(MC.Save(Internal::Types::Table()));
    MC.Print();
    std::cin.get();
    Foo = MC.Save(Internal::Types::Table());
    MC.Print();
    std::cin.get();
    Foo = MC.Save(Internal::Types::Table());
    MC.Print();
    std::cin.get();
    }

    std::vector<CountedReference> Refs = {MC.Save(Internal::Types::Table()), MC.Save(Internal::Types::Table()), MC.Save(Internal::Types::Table())};

    MC.Print();
    //Test if the refcount increases properly:
    std::cout << "A second Ref points at value 2. It's Refcount should increase by one:" << std::endl;
    std::cin.get();
    CountedReference Ref = Refs[1];
    MC.Print();
    assert(MC.GetRefCount(Ref) == 2);
    //Test if all Values with a refcount of 0 get deleted. One value should survive, because two refs are pointing at him
    std::cout << "The vector with the refs of the three values gets cleared. All but one should be deleted:\n";
    std::cin.get();
    Refs.clear();
    MC.Print();
    assert(MC.Size() == 1);
    //Test if a Weakref does not increase the refcount of a value:
    std::cout << "Creating a weakref pointing to the remaining value. The refcount should not increase:\n";
    std::cin.get();
    Reference Weakref = Ref.GetWeakReference();
    MC.Print();
    assert(MC.GetRefCount(Ref) == 1);
    //Test if a CountedReference created from a WeakReference increases the Refcount by one
    std::cout << "Creating a CountedReference from a the WeakReference. The Refcount should increase by one:\n";
    std::cin.get();
    CountedReference Ref2(Weakref);
    MC.Print();
    assert(MC.GetRefCount(Ref2) == 2);
    //Test if the value gets properly decrefed when a WeakRef is assigned to a CountedRef
    std::cout << "Saving a new value and assigning a Weakref to the newly created CountedRef. The refcount of the new value should first be one...\n";
    std::cin.get();
    CountedReference Ref3 = MC.Save(Internal::Types::Table());
    MC.Print();
    assert(MC.GetRefCount(Ref3) == 1);
    std::cin.get();
    std::cout << " and in the next step get destroyed:\n";
    std::cin.get();
    Ref3 = Weakref;
    MC.Print();
    assert(MC.GetRefCount(Ref3) == 3 && MC.Size() == 1);
    std::cin.get();
    CountedReference Ref4 = Weakref;
    std::cin.get();
    //Test nullreferences
    std::cout << "Creating a nullreference. It should be null:\n";
    std::cin.get();
    Reference Ref5;
    assert(Ref5.IsNull());
    std::cout << "Assigning a weakref to the newly created nullref. It must not be null anymore:\n";
    std::cin.get();
    Ref5 = Ref4.GetWeakReference();
    assert(!Ref5.IsNull());
    std::cin.get();
    std::cout << "Testint assignments:\n";
    MC.Print();
    CountedReference Ref6 = Ref4 = MC.Save(Internal::Types::Table());
    MC.Print();
    std::cin.get();
    Ref6 = MC.Save(Internal::Types::Table());
    MC.Print();
    std::cin.get();
    assert(Ref6->KeySize() == 0);
    //*
    /*
    CountedReference Ref = MC.Save(12ll);
    Reference WeakRef = MC.Save(1234ll).GetWeakReference();
    CountedReference Ref2 = MC.Save(123ll);
    MC.Print();
    CountedReference Ref3(Ref2);
    MC.Print();//*/
    return 0;
}
