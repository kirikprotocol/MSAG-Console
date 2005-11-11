#ident "$Id$"
#ifndef __SMSC_INMAN_CONSOLE_H__
#define __SMSC_INMAN_CONSOLE_H__

#include <vector>
#include <string>
#include <list>
#include <sstream>
#include <iostream>

namespace smsc {
namespace inman {
namespace common {

class Console
{
public:

    enum console_item_type_t
    {
        CTYPE_UCHAR,
        CTYPE_CHAR,
        CTYPE_UINT,
        CTYPE_INT,
        CTYPE_FLOAT,
        CTYPE_STRING,
        CTYPE_FUNCTION
    };

    typedef void (*console_function)(Console&, const std::vector<std::string> &);

    typedef struct
    {
        std::string         name;
        console_item_type_t type;
        union
        {
            console_function function;
            void *var;
        };
    } console_item_t;

public:
    Console();
    virtual ~Console();

    virtual void addItem(const std::string& command, console_function function);
    virtual void addItem(const std::string& command, void *pointer, console_item_type_t type);
    virtual void deleteItem(const std::string & strName);
    virtual void run(const std::string& prompt);

private:
    virtual void parse(const std::string& commandLine);
    typedef std::list<console_item_t> ITEM_DB;
    ITEM_DB m_itemList;
    bool    m_exit;
    friend void console_exit(Console&, const std::vector<std::string> &);
    friend void console_help(Console&, const std::vector<std::string> &);
};

} // common
} // inman
} // smsc

#endif /* __SMSC_INMAN_CONSOLE_H__ */

