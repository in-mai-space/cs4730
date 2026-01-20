#include "Book.h"

// PART 1
Book::Book(const std::string t, int id) : title(t), id(id) {
    std::cout << "[Book created]: \"" << title << "\" (id: " << id << ")" 
	    << std::endl;
}
    
Book::~Book() {
    std::cout << "[Book destroyed]: \"" << title << "\" (id: " << std::to_string(id) 
	    << ")" << std::endl;
}
    
void Book::Read() {
    std::cout << "\"" << title << "\" (id: " << std::to_string(id) << ")" << std::endl;
}


// Utility function for printing
void printTwoBooks(std::unique_ptr<Book> &aliceBook, 
        std::unique_ptr<Book> &bobBook) {
    printTwoBooks(aliceBook.get(), bobBook.get());
}

void printTwoBooks(std::shared_ptr<Book> &aliceBook, 
        std::shared_ptr<Book> &bobBook) {
    printTwoBooks(aliceBook.get(), bobBook.get());
}

void printTwoBooks(Book *aliceBook, Book *bobBook) {
     if (aliceBook) {
         std::cout << "- Alice: ";
	 aliceBook->Read();
    } else {
         std::cout << "- Alice: no book" << std::endl;
    }
    if (bobBook) {
         std::cout << "- Bob: ";
	 bobBook->Read();
    } else {
         std::cout << "- Bob: no book" << std::endl;
    }
}
