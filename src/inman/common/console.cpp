#include "console.hpp"

#include <stdlib.h>

namespace smsc {
namespace inman {
namespace common {

void console_exit(Console& console, const std::vector<std::string> &)
{
    console.m_exit = true;
}

void console_help(Console& console, const std::vector<std::string> &)
{
    std::cout << "Available commands: " << std::endl;
    std::list<Console::console_item_t>::const_iterator iter;
    for (iter = console.m_itemList.begin(); iter != console.m_itemList.end(); ++iter)
        std::cout << (*iter).name << " ";
    std::cout << std::endl;
}

Console::Console()
{
    addItem("exit", &console_exit);
    addItem("help", &console_help);
}

Console::~Console()
{
}

void Console::run(const std::string& prompt)
{
    m_exit = false;
    while (!m_exit) {
        char line[128];
        std::cout << prompt.c_str();
        std::cin.getline( line, sizeof(line) );
        try {
    	    parse( line );
        } catch(const std::exception& ex) {
    	    std::cout << "ERROR: " << ex.what() << std::endl;
        }
    }         
}

void Console::parse(const std::string& commandLine)
{
    if (commandLine.empty())
        return;

    std::string::size_type index = 0;
    std::vector<std::string> arguments;
    std::list<console_item_t>::const_iterator iter;

    while (index != std::string::npos) {
        std::string::size_type next_space = commandLine.find(' ', index);
        std::string token = commandLine.substr(index, next_space - index);
        if (!token.empty())
            arguments.push_back( token );
        if (next_space != std::string::npos)
            index = next_space + 1;
        else
            break;
    }
    if (!arguments.size())
        return;

    // execute
    for (iter = m_itemList.begin(); iter != m_itemList.end(); ++iter) {
        if (iter->name == arguments[0]) {
            switch(iter->type) {
            case CTYPE_UCHAR:
                if (arguments.size() == 1)
                    std::cout << (*iter).name << " = " << *((unsigned char *)(*iter).var) << std::endl;
                else if (arguments.size() == 2)
                    *((unsigned char *)(*iter).var) = (unsigned char) atoi(arguments[1].c_str());
                return;

            case CTYPE_CHAR:
                if (arguments.size() == 1)
                    std::cout << (*iter).name << " = " << *((char *)(*iter).var) << std::endl;
                else if (arguments.size() == 2)
                    *((char *)(*iter).var) = (char) atoi(arguments[1].c_str());
                return;

            case CTYPE_UINT:
                if (arguments.size() == 1)
                    std::cout << (*iter).name << " = " << *((unsigned int *)(*iter).var) << std::endl;
                else if (arguments.size() == 2)
                    *((unsigned int *)(*iter).var) = (unsigned int) atoi(arguments[1].c_str());
                return;

            case CTYPE_INT:
                if (arguments.size() == 1)
                    std::cout << (*iter).name << " = " << *((int *)(*iter).var) << std::endl;
                else if (arguments.size() == 2)
                    *((int *)(*iter).var) = (int) atoi(arguments[1].c_str());
                return;

            case CTYPE_FLOAT:
                if (arguments.size() == 1)
                    std::cout << (*iter).name << " = " << *((float *)(*iter).var) << std::endl;
                else if (arguments.size() == 2)
                    *((float *)(*iter).var) = (float)atof(arguments[1].c_str());
                return;

            case CTYPE_STRING:
                if (arguments.size() == 1)
                    std::cout << (*iter).name << " = " << (std::string *)(*iter).var << std::endl;
                else {
                    *((std::string *)(*iter).var) = "";         // reset variable
                    for (int i = 0; i < arguments.size(); ++i)   // add new string
                        *((std::string *)(*iter).var) += arguments[i];
                }
                return;

            case CTYPE_FUNCTION:
                (*iter).function(*this, arguments);
                return;
            }
        }
    }

    std::cerr << "Unknown command: " << arguments[0] << std::endl;
    return;
}

void Console::addItem(const std::string& command, console_function function)
{
    addItem( command, (void*) function, CTYPE_FUNCTION );
}

void Console::addItem(const std::string & strName, void *pointer, console_item_type_t type)
{
    console_item_t it;

    // fill item properties
    it.name = strName;
    it.type = type;

    // address
    if (type != CTYPE_FUNCTION)
        it.var = pointer;
    else
        it.function = (console_function) pointer;

    // add item
    m_itemList.push_front(it);
}

void Console::deleteItem(const std::string & strName)
{
    ITEM_DB::iterator iter;

    // find item
    for (iter = m_itemList.begin(); iter != m_itemList.end(); ++iter) {
        if (iter->name == strName) {
            m_itemList.erase(iter);
            break;
        }
    }
}

} //common
} //inman
} //smsc
