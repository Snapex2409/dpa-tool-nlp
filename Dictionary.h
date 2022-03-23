#pragma once

#include<sys/stat.h>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include <fstream>

/*
* 0: Noun := A noun is a word that functions as the name of a specific object or set of objects, such as living creatures, places, actions, qualities, states of existence, or ideas.
* 1: Verb := A verb is a word that in syntax conveys an action, an occurrence, or a state of being.
* 2: Adjective := In linguistics, an adjective is a word that modifies a noun or noun phrase or describes its referent. Its semantic role is to change information given by the noun.
* 3: Adverb := An adverb is a word or an expression that modifies a verb, adjective, another adverb, determiner, clause, preposition, or sentence. Adverbs typically express manner, place, time, frequency, degree, level of certainty, etc., answering questions such as how?, in what way?, when?, where?, and to what extent.
* 4: Pronoun := In linguistics and grammar, a pronoun is a word or a group of words that one may substitute for a noun or noun phrase.
* 5: Preposition := Prepositions and postpositions, together called adpositions, are a class of words used to express spatial or temporal relations or mark various semantic roles.
* 6: Conjunction := In grammar, a conjunction is a part of speech that connects words, phrases, or clauses that are called the conjuncts of the conjunctions.
* 7: Interjection := An interjection is a word or expression that occurs as an utterance on its own and expresses a spontaneous feeling or reaction. It is a diverse category, encompassing many different parts of speech, such as exclamations (ouch!, wow!), curses (damn!), greetings, response particles, hesitation.
* 8: Article := An article is any member of a class of dedicated words that are used with noun phrases to mark the identifiability of the referents of the noun phrases. The category of articles constitutes a part of speech.
* 9: Name := Name of beings.
* Descriptions from: https://en.wikipedia.org/wiki/Part_of_speech
*/

namespace Dict {

#define PART_TEXT parts[0]
#define PART_CLASS parts[1]


	enum WordClass
	{
		NOUN, VERB, ADJECTIVE, ADVERB, PRONOUN, PREPOSITION, CONJUNCTION, INTERJECTION, ARTICLE, NAME, WORD_CLASS_SIZE
	};

	enum Buffers
	{
		OLD, NEW, BUFFERS_SIZE
	};

	static WordClass from_int(int i)
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

	struct DictionaryEntry
	{
		struct Text { int start; int length; Buffers buffer_id; } text;
		WordClass clazz;
		bool active;

	};

	bool operator== (const DictionaryEntry& a, const DictionaryEntry& b) 
	{
		return a.active == b.active && a.clazz == b.clazz && a.text.buffer_id == b.text.buffer_id && a.text.length == b.text.length && a.text.start == b.text.start;
	}

	/*N: Size of tmp entries*/
	template<int N>
	class Dictionary
	{
	public:
		Dictionary(int start_size) : m_max_size(start_size), m_size(0), m_buffer(nullptr), m_buffer_last(nullptr), m_buffer_raw(nullptr), m_last_delete(false), m_new_raw_size(0), m_new_raw(nullptr)
		{
			m_buffer = new DictionaryEntry[start_size];
			memset(m_buffer, 0, sizeof(DictionaryEntry) * start_size);
			m_buffer_last = m_buffer;
		}
		~Dictionary()
		{
			if (m_buffer_raw) delete[] m_buffer_raw;
			if (m_buffer)  delete[] m_buffer;
			if (m_new_raw) delete[] m_new_raw;
		}
		
		/*inserts the specified entry*/
		void insert(const std::string& str, WordClass clazz) 
		{
			int str_size = str.size();

			//create new raw buffer and append new string
			char* tmp = new char[m_new_raw_size + str_size+1];
			memcpy(tmp, m_new_raw, m_new_raw_size);
			memcpy(tmp + m_new_raw_size, str.c_str(), str_size);
			if (m_new_raw) delete[] m_new_raw;
			m_new_raw = tmp;
			m_new_raw_size += str_size;
			
			auto it = m_new.begin();
			auto ite = m_new.end();
			while ( it != ite && strncmp(p_get_raw(it->text), str.c_str(), it->text.length < str.length() ? it->text.length : str.length()) <= 0) it++;
			m_new.insert(it, { { m_new_raw_size - str_size, str_size, Buffers::NEW }, clazz, true });

			if (m_new.size() >= N) p_pack();
		}

		/*searches for the specified entry, returns the first found result*/
		const DictionaryEntry* find(std::string& str) 
		{
			DictionaryEntry* res = nullptr;
			DictionaryEntry* ptr = p_find(str);
			if (ptr == -1) 
			{
				for (DictionaryEntry* e : m_new)
				{
					if (strncmp(p_get_raw(e->text), str.c_str(), e->text.length < str.size() ? e->text.length : str.size()) == 0 && e->active) res = e;
				}
			} 
			else res = ptr;
			if (m_last_delete) p_cleanup();
			return res;
		}

		/*searches for the specified entry, returns the first found result*/
		const DictionaryEntry* operator[](const std::string& str) 
		{
			return find(str);
		}

		/*removes specified entry*/
		void remove(const std::string& str) 
		{
			DictionaryEntry* ptr = p_find(str);
			if (ptr) 
			{
				if(ptr == (DictionaryEntry*) - 1)
				{
					auto f = [&]() {
						for (DictionaryEntry& e : m_new)
						{
							if (strncmp(p_get_raw(e.text), str.c_str(), e.text.length < str.length() ? e.text.length : str.length()) == 0 && e.active) return e;
						}
					};
					
					const DictionaryEntry& val = f();
					m_new.remove(val);
				}
				else ptr->active = false;
				m_last_delete = true;
			}
		}

		/*
		* writes all entries to the specified file
		*/
		void write_dictionary(const std::string& path)
		{
			p_cleanup();	//remove deleted entries
			p_pack();		//merge entries

			std::ofstream file(path);
			if (!file.is_open()) { std::cout << "Unable to open file." << std::endl; return; }

			for (int index = 0; index < m_size; index++) 
			{
				const DictionaryEntry& entry = m_buffer[index];
				char* txt = p_get_raw(entry.text);
				char old_char = txt[entry.text.length];
				txt[entry.text.length] = 0;
				file << txt << ";" << entry.clazz << std::endl;
				txt[entry.text.length] = old_char;
			}
			file.close();
		}

		/*
		* loads all entries from the specified file
		* dictionary must be sorted by text asc
		*/
		void load_dictionary(const std::string& path) 
		{
			if (m_buffer_raw != nullptr) delete[] m_buffer_raw;
			FILE* file;
			fopen_s(&file, path.c_str(), "r");
			if (file == static_cast<FILE*>(0)) { std::cout << "Failed to open dictionary" << std::endl; return; }
			struct stat status = { 0 };
			if (stat(path.c_str(), &status)) perror("Failed to load dictionary");
			m_buffer_raw = new char[status.st_size+1];
			size_t read_bytes = 0;
			size_t total_read_bytes = 0;
			while (read_bytes = fread(m_buffer_raw + total_read_bytes, 1, status.st_size, file)) total_read_bytes += read_bytes;

			//parse all lines
			for (int start = 0, end = 0; end <= status.st_size; end++)
			{
				if (m_buffer_raw[end] != '\n') continue;
				//split line into parts, delimiter is ";"
				std::vector<std::string_view> parts;
				int pos_s = start, pos_e = start;
				for (; pos_e < end; pos_e++)
				{
					if (m_buffer_raw[pos_e] != ';') continue;
					parts.emplace_back(m_buffer_raw + pos_s, m_buffer_raw + pos_e);
					pos_s = pos_e + 1;
				}
				parts.emplace_back(m_buffer_raw + pos_s, m_buffer_raw + end);
				start = end + 1;
				//handle parts
				p_append(parts);
			}
			fclose(file);
		}
	private:
		/*storage of raw text of dictionary*/
		char* m_buffer_raw;
		/*sorted entries*/
		DictionaryEntry* m_buffer;
		/*next free position*/
		DictionaryEntry* m_buffer_last;
		/*buffer for temporary changes*/
		std::list<DictionaryEntry> m_new;
		/*storage of raw text of new dict entries*/
		char* m_new_raw;
		/*size of raw text buffer of new dict entries*/
		int m_new_raw_size;
		/*current size of main dict*/
		int m_size;
		/*current maximal size of main dict*/
		int m_max_size;
		/*was the last action a deletion*/
		bool m_last_delete;

		/*extends the entry buffer by size*/
		void p_extend(int size) 
		{
			DictionaryEntry* tmp = new DictionaryEntry[m_max_size + size];
			memset(tmp, 0, sizeof(DictionaryEntry) * m_max_size + size);
			memcpy(tmp, m_buffer, sizeof(DictionaryEntry) * m_max_size);
			delete[] m_buffer;
			m_buffer = tmp;
			m_buffer_last = m_buffer + m_size;
			m_max_size += size;
		}

		/*appends the new entry to the end of the array*/
		void p_append(std::vector<std::string_view>& parts) 
		{
			if (m_size == m_max_size) p_extend(m_max_size);
			m_buffer_last->text = { static_cast<int>(PART_TEXT.data() - m_buffer_raw), static_cast<int>(PART_TEXT.size()), Buffers::OLD};
			m_buffer_last->clazz = from_int(atoi(PART_CLASS.data()));
			m_buffer_last->active = true;
			m_buffer_last++;
			m_size++;
		}

		/*merges tmp buffer with main buffer*/
		void p_pack() 
		{
			//increase max size of buffer if needed
			if (m_new.size() + m_size > m_max_size) m_max_size *= 2;
			DictionaryEntry* tmp = new DictionaryEntry[m_max_size];
			memset(tmp, 0, sizeof(DictionaryEntry) * m_max_size);
			int old_size = m_size;
			m_size += m_new.size();

			int offset = 0;
			int index = 0;
			while (m_new.size() > 0 && index < old_size) //still entries in both lists
			{
				//get entries
				DictionaryEntry& a = m_new.front();
				DictionaryEntry& b = m_buffer[index];

				//which one is shorter
				bool a_shorter = a.text.length < b.text.length;
				DictionaryEntry& sht = a_shorter ? a : b;
				DictionaryEntry& lng = a_shorter ? b : a;

				int cmp = strncmp(p_get_raw(sht.text), p_get_raw(lng.text), a_shorter ? a.text.length : b.text.length);

				if(cmp > 0) //shorter one is after
				{
					if (a_shorter) 
					{
						tmp[offset++] = b;
						index++;
					}
					else
					{
						tmp[offset++] = a;
						m_new.pop_front();
					}
				}
				else //shorter one is before
				{
					if (a_shorter)
					{
						tmp[offset++] = a;
						m_new.pop_front();
					}
					else
					{
						tmp[offset++] = b;
						index++;
					}
				}
			}
			if (m_new.size() > 0) //still entries in new list
			{
				for (const DictionaryEntry& e : m_new) tmp[offset++] = e;
				m_new.clear();
			}
			else if (index < old_size) //still entries in real list
			{
				memcpy(tmp + offset, m_buffer + index, (old_size - index)*sizeof(DictionaryEntry));
				offset += old_size - index;
			}

			delete[] m_buffer;
			m_buffer = tmp;
			m_buffer_last = m_buffer + offset;
		}

		/*compares two dictionary entries*/
		int dictcmp(const DictionaryEntry& a, const DictionaryEntry& b)
		{
			//which one is shorter
			const DictionaryEntry& sht = (a.text.length < b.text.length) ? a : b;
			const DictionaryEntry& lng = (a.text.length >= b.text.length) ? a : b;

			return strncmp(p_get_raw(sht.text), p_get_raw(lng.text), sht.text.length);
		}

		/*removes deleted entries from main buffer*/
		void p_cleanup() 
		{
			DictionaryEntry* tmp = new DictionaryEntry[m_max_size];
			memset(tmp, 0, sizeof(DictionaryEntry) * m_max_size);

			const int end_index = m_size;
			m_size = 0;
			for (int index = 0; index < end_index; index++)
			{
				if (!m_buffer[index].active) continue;
				tmp[m_size++] = m_buffer[index];
			}
			delete[] m_buffer;
			m_buffer = tmp;
			m_buffer_last = m_buffer + m_size;
		}

		/* find internal
		* returns pointer to entry in array if found
		* returns -1 if in tmp list
		* returns 0 if not found
		*/
		DictionaryEntry* p_find(const std::string& str) 
		{
			for (DictionaryEntry& e : m_new) 
			{
				if (strncmp(p_get_raw(e.text), str.c_str(), e.text.length < str.length() ? e.text.length : str.length()) == 0 && e.active) return (DictionaryEntry*) - 1;
			}
			for (int index = 0; index < m_size; index++) 
			{
				if (strncmp(p_get_raw(m_buffer[index].text), str.c_str(), m_buffer[index].text.length < str.length() ? m_buffer[index].text.length : str.length()) == 0 && m_buffer[index].active) return m_buffer + index;
			}
			return 0;
		}

		/*finds the string section of the given text struct*/
		inline char* p_get_raw(const struct DictionaryEntry::Text& txt) {
			return (txt.buffer_id == Buffers::OLD) ? m_buffer_raw + txt.start : m_new_raw + txt.start;
		}
	};
}
