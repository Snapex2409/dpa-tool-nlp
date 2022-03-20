// NLP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Dictionary.h"

int main()
{
    Dict::Dictionary<10> dict(8);
    dict.insert("testa", Dict::WordClass::NAME);
    dict.insert("testb", Dict::WordClass::NAME);
    dict.insert("testc", Dict::WordClass::NAME);
    dict.insert("testd", Dict::WordClass::NAME);
    dict.insert("teste", Dict::WordClass::NAME);
    dict.insert("testf", Dict::WordClass::NAME);
    dict.insert("testg", Dict::WordClass::NAME);
    dict.insert("testh", Dict::WordClass::NAME);
    dict.insert("testi", Dict::WordClass::NAME);
    dict.insert("testj", Dict::WordClass::NAME);
    dict.insert("testk", Dict::WordClass::NAME);
    dict.insert("testl", Dict::WordClass::NAME);
    dict.insert("testm", Dict::WordClass::NAME);
    dict.insert("testn", Dict::WordClass::NAME);
    dict.insert("testo", Dict::WordClass::NAME);
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
