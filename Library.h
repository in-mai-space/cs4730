#ifndef __LIBRARY_H__
#define __LIBRARY_H__

#include <iostream>
#include <memory>
#include <string>
#include <list>

#include "Book.h"

// PART 1 - 3
class Library {
private:
    std::list<std::unique_ptr<Book> > books;
    
public:
    Library() {};
    ~Library() {};
    
    int Size(); 
    void ListBooks(); 
   
    void AddBook(std::unique_ptr<Book> book);    
    std::unique_ptr<Book> CheckoutBook();    
};

#endif // end of #ifndef __LIBRARY_H__
