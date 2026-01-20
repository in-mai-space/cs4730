#include <iostream>
#include <list>
#include <memory>
#include <string>

#include "Book.h"
#include "Library.h"

/*******************************************************************************
    PART 1 - 1
*******************************************************************************/

// TODO: Add a single line that transfers the book from
// Alice to Bob to make this function output the same
// as part_1_1_unique_ptr_base() in part_1_main.cpp
void part_1_1_unique_ptr() {
  std::unique_ptr<Book> aliceBook(new Book("C++ Book", 1));
  std::unique_ptr<Book> bobBook = nullptr;

  printTwoBooks(aliceBook, bobBook);

  std::cout << "\nAlice gave the book to Bob.\n" << std::endl;

  printTwoBooks(aliceBook, bobBook);
}

/*******************************************************************************
    PART 1 - 2
*******************************************************************************/

// TODO: Add a single line that shares Alice's book with
// Bob to make this function output the same
// as part_1_2_shared_ptr_base() in part_1_main.cpp
void part_1_2_shared_ptr() {
  std::shared_ptr<Book> aliceBook(new Book("C++ Book", 1));
  std::shared_ptr<Book> bobBook = nullptr;

  printTwoBooks(aliceBook, bobBook);

  std::cout << "\nAlice shared the book with Bob.\n" << std::endl;

  printTwoBooks(aliceBook, bobBook);
}

/*******************************************************************************
    PART 1 - 3
*******************************************************************************/

// Implement the following two member functions for the
// Library class. Look at Library.h and Library.cpp for other details
// about the Library class.
void Library::AddBook(std::unique_ptr<Book> book) {
  // TODO: push the book which is given as the function argument
  // into the BACK of the book list (i.e., books).
}

std::unique_ptr<Book> Library::CheckoutBook() {
  // TODO: take the book at the FRONT of the book list,
  // remove the book from the list, and return.
  // If there is no book in the list, return nullptr.
  // You may change the code within this function (i.e., return nullptr;)
  // however you want.

  return nullptr;
}
