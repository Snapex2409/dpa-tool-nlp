// NLP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Dictionary.h"

int main()
{
    Dict::Dictionary<10> dict(8);
    dict.insert("test0", Dict::WordClass::NAME);
    dict.insert("test1", Dict::WordClass::NAME);
    dict.insert("test2", Dict::WordClass::NAME);
    dict.insert("test3", Dict::WordClass::NAME);
    dict.insert("test4", Dict::WordClass::NAME);
    dict.insert("test5", Dict::WordClass::NAME);
    dict.insert("test6", Dict::WordClass::NAME);
    dict.insert("test7", Dict::WordClass::NAME);
    dict.insert("test8", Dict::WordClass::NAME);
    dict.insert("test9", Dict::WordClass::NAME);
    dict.insert("test10", Dict::WordClass::NAME);
    dict.insert("test11", Dict::WordClass::NAME);
    dict.insert("test12", Dict::WordClass::NAME);
    dict.insert("test13", Dict::WordClass::NAME);
    dict.insert("test14", Dict::WordClass::NAME);
    dict.write_dictionary("test.dict");
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
