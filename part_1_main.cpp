#include <iostream>
#include <memory>
#include <string>

#include "Book.h"
#include "Library.h"

// Prototypes of what you will write in part_1_your_task.cpp
void part_1_1_unique_ptr();
void part_1_2_shared_ptr();


/*******************************************************************************
    PART 1 - 1
*******************************************************************************/
void part_1_1_unique_ptr_base() {
    
    Book *aliceBook =  new Book("C++ Book", 1);
    Book *bobBook = nullptr;
    
    printTwoBooks(aliceBook, bobBook);

    std::cout << "\nAlice gave the book to Bob.\n" << std::endl;
    bobBook = aliceBook;
    aliceBook = nullptr;

    printTwoBooks(aliceBook, bobBook);

    delete bobBook;
}


/*******************************************************************************
    PART 1 - 2
*******************************************************************************/
void part_1_2_shared_ptr_base() {
    
    Book *aliceBook =  new Book("C++ Book", 1);
    Book *bobBook = nullptr;
    
    printTwoBooks(aliceBook, bobBook);
    
    std::cout << "\nAlice shared the book with Bob.\n" << std::endl;
    bobBook = aliceBook;

    printTwoBooks(aliceBook, bobBook);

    delete bobBook;
}


/*******************************************************************************
    PART 1 - 3
*******************************************************************************/
void part_1_3() {
    Library library;
    std::unique_ptr<Book> checkedoutBook;

    // Adds three books to the library
    for (int i = 0; i < 3; i++) {
        std::unique_ptr<Book> book(new Book("C++ Book", i));
        library.AddBook(std::move(book));
        std::cout << "Book added to library\n";
    }

    // List books in the library
    std::cout << "\nLibrary contains " << library.Size() << " books:\n";
    library.ListBooks();
    std::cout << std::endl;

    // Check out one book and read the book
    checkedoutBook = library.CheckoutBook();
    if (!checkedoutBook) {
        std::cout << "No book to check out." << std::endl;
    } else {
        std::cout << "Book checked out: ";
        checkedoutBook->Read();
    }

    // List books in the library
    std::cout << "\nLibrary contains " << library.Size() << " books:\n";
    library.ListBooks();
    std::cout << std::endl;
}


/*******************************************************************************
    Main function
*******************************************************************************/

int main() {

    std::cout << "\n== Part 1-1 unique_ptr base ==\n" << std::endl;
    part_1_1_unique_ptr_base();

    std::cout << "\n== Part 1-1 unique_ptr your task: must match base output ==\n" << std::endl;
    part_1_1_unique_ptr();

    std::cout << "\n== Part 1-2 shared_ptr base ==\n" << std::endl;
    part_1_2_shared_ptr_base();

    std::cout << "\n== Part 1-2 shared_ptr your task: must match base output ==\n" << std::endl;
    part_1_2_shared_ptr();

    std::cout << "\n== Part 1-3 working with list and unique_ptr: make it work properly ==\n" << std::endl;
    part_1_3();

    return 0;
}


