#pragma once

#include<sys/stat.h>
#include <string>
#include <vector>
#include <list>
#include <algorithm>

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


	enum class WordClass
	{
		NOUN, VERB, ADJECTIVE, ADVERB, PRONOUN, PREPOSITION, CONJUNCTION, INTERJECTION, ARTICLE, NAME, WORD_CLASS_SIZE
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
		std::string_view text;
		WordClass clazz;
		bool active;
	};

	/*N: Size of tmp entries*/
	template<int N>
	class Dictionary
	{
	public:
		Dictionary(int start_size) : m_max_size(start_size), m_size(0), m_buffer(nullptr), m_buffer_last(nullptr), m_buffer_raw(nullptr), m_last_delete(false)
		{
			m_buffer = new DictionaryEntry[start_size];
			memset(m_buffer, 0, sizeof(DictionaryEntry) * start_size);
			m_buffer_last = m_buffer;
		}
		~Dictionary()
		{
			delete[] m_buffer;
		}
		
		/*inserts the specified entry*/
		void insert(const std::string& str, WordClass clazz) 
		{
			m_new_raw.insert(0, str);
			auto it = m_new_raw.begin();
			m_new.emplace_back(std::string_view(it, it + str.size() - 1), clazz, true);
			if (m_new.size() >= N) p_pack();
		}

		/*searches for the specified entry*/
		const DictionaryEntry* find(std::string& str) 
		{
			DictionaryEntry* res = nullptr;
			DictionaryEntry* ptr = p_find(str);
			if (ptr == -1) 
			{
				for (DictionaryEntry* e : m_new)
				{
					if (strcmp(e->text, str) == 0 && e->active) res = e;
				}
			} 
			else res = ptr;
			if (m_last_delete) p_cleanup();
			return res;
		}

		const DictionaryEntry* operator[](std::string str) 
		{
			return find(str);
		}

		/*removes specified entry*/
		void remove(std::string& str) 
		{
			DictionaryEntry* ptr = p_find(str);
			if (ptr) 
			{
				if(ptr == -1) 
				{
					auto f = []() {
						for (DictionaryEntry* e : m_new)
						{
							if (strcmp(e->text.data(), str.c_str()) == 0 && e->active) return e;
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
		* loads all entries from the specified file
		* dictionary must be sorted by text asc
		*/
		void load_dictionary(std::string& path) 
		{
			if (m_buffer_raw != nullptr) delete[] m_buffer_raw;

			FILE* file = fopen(path.c_str(), "r");
			struct stat status = { 0 };
			if (stat(path.c_str(), &status)) perror("Failed to load dictionary");
			m_buffer_raw = new char[status.st_size];
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
					parts.emplace_back(m_buffer_raw + pos_s, m_buffer_raw + pos_e - 1);
					pos_s = pos_e + 1;
				}
				parts.emplace_back(m_buffer_raw + pos_s, m_buffer_raw + end - 1);

				//handle parts
				p_append(parts);
			}
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
		std::string m_new_raw;
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
			m_buffer_last->text = PART_TEXT;
			m_buffer_last->clazz = from_int(atoi(PART_CLASS.data()));
			m_buffer_last->active = true;
			m_buffer_last++;
		}

		/*merges tmp buffer with main buffer*/
		void p_pack() 
		{
			//increase max size of buffer if needed
			if (m_new.size() + m_size > m_max_size) m_max_size *= 2;
			DictionaryEntry* tmp = new DictionaryEntry[m_max_size];
			memset(tmp, 0, sizeof(DictionaryEntry) * m_max_size);

			m_size += m_new.size();

			int offset = 0;
			int last = 0;
			int index = 0;
			while (m_new.size() > 0) 
			{
				DictionaryEntry& first = m_new.front();
				while (strcmp(m_buffer[index].text.data(), first.text.data()) < 0) index++;
				memcpy(tmp + offset, m_buffer + last, sizeof(DictionaryEntry) * (index - last));
				last = index;
				offset += (index - last);
				tmp[offset] = first;
				offset++;
				m_new.pop_front();
			}

			delete[] m_buffer;
			m_buffer = tmp;
			m_buffer_last = m_buffer + offset;
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
		DictionaryEntry* p_find(std::string& str) 
		{
			for (DictionaryEntry* e : m_new) 
			{
				if (strcmp(e->text.data(), str.c_str()) == 0 && e->active) return -1;
			}
			for (int index = 0; index < m_size; index++) 
			{
				if (strcmp(m_buffer[index].text.data(), str.c_str()) == 0 && m_buffer[index].active) return m_buffer + index;
			}
			return 0;
		}
	};
}
