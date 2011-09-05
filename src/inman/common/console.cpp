#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#define _DEBUG_PARSING

#include "inman/common/console.hpp"
#include "inman/common/CmdLine.hpp"

#include <string.h>
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
        char line[256];
        std::cout << prompt.c_str();
//        std::cin.getline( line, sizeof(line) );
        if (fgets(line, sizeof(line)-1, stdin)) {
          size_t n = strlen(line);
          if (line[n - 1] == '\n')
            line[n - 1] = 0;
          try {
            parse(line);
          } catch(const std::exception& ex) {
            std::cout << "ERROR: " << ex.what() << std::endl;
          }
        }
    }         
}

void Console::parse(const std::string& commandLine)
{
    if (commandLine.empty())
        return;

    std::vector<std::string> arguments;
    ParsingResult res = parseCmdLine(commandLine.c_str(), arguments);
    if (!res.isOk()) {
      if (res._status == ParsingResult::rcMatch) {
        std::cerr << "Unmatched delimiter at pos: " << res._pos << std::endl;
      } else if (res._status == ParsingResult::rcEscape) {
        std::cerr << "Invalid escape sequence at pos: " << res._pos << std::endl;
      }
      return;
    }

    if (arguments.empty())
        return;

#ifdef _DEBUG_PARSING
    for (std::vector<std::string>::const_iterator
      it = arguments.begin(); it != arguments.end(); ++it) {
      //fprintf(stderr, "arg: '%s'\n", it->c_str());
      std::cerr << "arg: \'" << *it << "\'" << std::endl;
    }
#endif /* _DEBUG_PARSING */

    // execute
    std::list<console_item_t>::const_iterator iter;
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
                    for (size_t i = 0; i < arguments.size(); ++i)   // add new string
                        *((std::string *)(*iter).var) += arguments[i];
                }
                return;

            case CTYPE_FUNCTION:
                (*iter).function(*this, arguments);
                return;
            }
        }
    }

    std::cerr << "Unknown command: '" << arguments[0] << "'" << std::endl;
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
