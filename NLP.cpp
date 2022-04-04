// NLP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <string_view>
#include "Dictionary.h"
#include "CLI.h"



static void print_help();
template <int N>
static void edit_dict(Dict::Dictionary<N>& dict);

int main(int argc, char** argv)
{
    CLI::InputParser input(argc, argv);
    Dict::Dictionary<10> dict(8);

    bool output = false;
    std::string output_path = "";

    if (cmde("-h", "--help")) { print_help(); return EXIT_SUCCESS; }
    if (cmde("-ld", "--load-dict")) { dict.load_dictionary(input.getCmdOption(input.cmdOptionExists("-ld") ? "-ld" : "--load-dict")); }
    if (cmde("-o", "-o")) { output = true; output_path = input.getCmdOption("-o"); }
    if (cmde("-e", "--edit")) 
    {
        edit_dict(dict);
        if (output) dict.write_dictionary(output_path);
        return EXIT_SUCCESS;
    }

    print_help();
}

static void print_help()
{
    
}

template<int N>
void edit_dict(Dict::Dictionary<N>& dict)
{
    std::string cmd;
    std::string s_buffer;
    int i_buffer;
    auto add = [&]()
    {
        std::cout << "text:";
        std::cin >> s_buffer;
        std::cout << std::endl;
        std::cout << "classes" << std::endl;
        for (i_buffer = 0; i_buffer < Dict::WORD_CLASS_SIZE; i_buffer++) std::cout << Dict::word_class_names[i_buffer] << ": " << i_buffer << " ";
        std::cout << std::endl;
        std::cout << "class:";
        std::cin >> i_buffer;
        std::cout << std::endl;

        dict.insert(s_buffer, Dict::from_int(i_buffer));
    };
    auto add_s = [&](const std::string& str)
    {
        std::cout << "classes" << std::endl;
        for (i_buffer = 0; i_buffer < Dict::WORD_CLASS_SIZE; i_buffer++) std::cout << Dict::word_class_names[i_buffer] << ": " << i_buffer << " ";
        std::cout << std::endl;
        std::cout << "class:";
        std::cin >> i_buffer;
        std::cout << std::endl;

        dict.insert(str, Dict::from_int(i_buffer));
    };

    while (true)
    {
        std::cout << "q : quit" << std::endl;
        std::cout << "a : add one entry" << std::endl;
        std::cout << "c : classify text" << std::endl;

        std::cin >> cmd;
        if (cmd == "q") return;
        if (cmd == "a") add();
        if (cmd == "c")
        {
            std::cout << "enter path to file:";
            std::cin >> s_buffer;
            std::cout << std::endl;

            //read file
            {
                FILE* file;
                fopen_s(&file, s_buffer.c_str(), "r");
                if (file == static_cast<FILE*>(0)) { std::cout << "Failed to open file." << std::endl; return; }
                struct stat status = { 0 };
                if (stat(s_buffer.c_str(), &status)) perror("Failed to load dictionary");
                char* buffer = new char[status.st_size + 1];
                memset(buffer, 0, status.st_size + 1);

                size_t read_bytes = 0;
                size_t total_read_bytes = 0;
                while (read_bytes = fread(buffer + total_read_bytes, 1, status.st_size, file)) total_read_bytes += read_bytes;
                for (int index = 0; index < status.st_size; index++)
                {
                    if (buffer[index] == '\n' || buffer[index] == '\t' || buffer[index] == '\r') buffer[index] = ' ';
                }
                fclose(file);
                s_buffer = std::string(buffer);
            }
            //convert to lowercase
            std::transform(s_buffer.begin(), s_buffer.end(), s_buffer.begin(),
                [](unsigned char c) { return std::tolower(c); });

            std::list<std::string> parts;
            {
                int start = 0, end = 0;
                for (; end < s_buffer.size(); end++)
                {
                    if (s_buffer.at(end) == ' ' ||
                        s_buffer.at(end) == '.' ||
                        s_buffer.at(end) == ',' ||
                        s_buffer.at(end) == '!' ||
                        s_buffer.at(end) == '?' ||
                        s_buffer.at(end) == ':' ||
                        s_buffer.at(end) == '\"')
                    {
                        if (start >= end) continue;
                        parts.push_back(s_buffer.substr(start, end - start));
                        for (int index = end; index < s_buffer.size(); index++)
                        {
                            if (s_buffer.at(index) != ' ' &&
                                s_buffer.at(index) != '.' &&
                                s_buffer.at(index) != ',' &&
                                s_buffer.at(index) != '!' &&
                                s_buffer.at(index) != '?' &&
                                s_buffer.at(index) != ':' &&
                                s_buffer.at(index) != '\"')
                            {
                                start = index;
                                break;
                            }
                        }
                    }
                }
                for (int index = start; index < s_buffer.size(); index++)
                {
                    if (s_buffer.at(index) == ' ' ||
                        s_buffer.at(index) == '.' ||
                        s_buffer.at(index) == ',' ||
                        s_buffer.at(index) == '!' ||
                        s_buffer.at(index) == '?' ||
                        s_buffer.at(index) == ':' ||
                        s_buffer.at(index) == '\"')
                    {
                        end = index;
                        break;
                    }
                }
                parts.push_back(s_buffer.substr(start, end - start));
            }
            for (const std::string& s : parts)
            {
                if (dict.find(s)) continue;
                loop: std::cout << "add \"" << s << "\" to dict? [Y/N]" << std::endl;
                std::cin >> s_buffer;
                if (s_buffer == "Y") add_s(s); else goto loop;
            }
        }
    }
}
