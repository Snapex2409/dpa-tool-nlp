#include "Dictionary.h"

Dict::WordClass Dict::from_int(int i)
{
	switch (i)
	{
	case 0: return Dict::WordClass::NOUN;
	case 1: return Dict::WordClass::VERB;
	case 2: return Dict::WordClass::ADJECTIVE;
	case 3: return Dict::WordClass::ADVERB;
	case 4: return Dict::WordClass::PRONOUN;
	case 5: return Dict::WordClass::PREPOSITION;
	case 6: return Dict::WordClass::CONJUNCTION;
	case 7: return Dict::WordClass::INTERJECTION;
	case 8: return Dict::WordClass::ARTICLE;
	case 9: return Dict::WordClass::NAME;
	default: return Dict::WordClass::WORD_CLASS_SIZE;
	}
}

bool Dict::operator== (const Dict::DictionaryEntry& a, const Dict::DictionaryEntry& b)
{
	return a.active == b.active && a.clazz == b.clazz && a.text.buffer_id == b.text.buffer_id && a.text.length == b.text.length && a.text.start == b.text.start;
}

std::string Dict::word_class_names[Dict::WORD_CLASS_SIZE] = { "noun", "verb", "adjective", "adverb", "pronoun", "preposition", "conjunction", "interjection", "article", "name" };