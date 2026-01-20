#include "Library.h"

// PART 1 - 3
int Library::Size() {
    return books.size();
}

void Library::ListBooks() {
    for (auto it = books.begin(); it != books.end(); it++) {
        (*it)->Read();
    }
}

