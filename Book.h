#ifndef __BOOK_H__
#define __BOOK_H__

#include <iostream>
#include <memory>
#include <string>

// PART 1
class Book {
private:
    std::string title;
    int id;
    
public:
    Book(const std::string t, int id);    
    ~Book();    
    void Read();
};


// Utility function for printing
void printTwoBooks(std::unique_ptr<Book> &aliceBook, 
        std::unique_ptr<Book> &bobBook);
void printTwoBooks(std::shared_ptr<Book> &aliceBook, 
        std::shared_ptr<Book> &bobBook);
void printTwoBooks(Book *aliceBook, Book *bobBook);

#endif // end of #ifndef __BOOK_H__
