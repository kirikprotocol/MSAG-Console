
#include "Messages.h"

namespace smsc { namespace mcisme
{

int Message::maxRowsPerMessage = 5;

const char* MSG_TEMPLATE_PARAM_ABONENT = "abonent";
const char* MSG_TEMPLATE_PARAM_CALLER  = "caller";
const char* MSG_TEMPLATE_PARAM_TOTAL   = "total";
const char* MSG_TEMPLATE_PARAM_COUNT   = "count";
const char* MSG_TEMPLATE_PARAM_DATE    = "date";
const char* MSG_TEMPLATE_PARAM_ROWS    = "rows";

static const char*  UNKNOWN_CALLER = "XXX";

void keyIsNotSupported(const char* key) throw(AdapterException) {
    throw AdapterException(Exception("Argument '%s' is not supported by message formatter", key));
}
void typeIsNotSupported(const char* type) throw(AdapterException) {
    throw AdapterException(Exception("Type '%s' is not supported by message formatter", type));
}
char* convertStrToLowerCase(const char* str)
{
    if (!str) return 0;
    char* low = new char[strlen(str)+1]; int cur = 0;
    while (*str) low[cur++] = (char)tolower(*str++);
    low[cur] = '\0';
    return low;
}
const char* findMessageTemplateKey(const char* key) throw(AdapterException)
{
    if (!key || !key[0]) throw AdapterException(Exception("Argument key is null"));
    std::auto_ptr<char> lkeyGuard(convertStrToLowerCase(key));
    const char* lkey = lkeyGuard.get();

    if      (!strcmp(MSG_TEMPLATE_PARAM_ABONENT, lkey)) return MSG_TEMPLATE_PARAM_ABONENT;
    else if (!strcmp(MSG_TEMPLATE_PARAM_CALLER,  lkey)) return MSG_TEMPLATE_PARAM_CALLER;
    else if (!strcmp(MSG_TEMPLATE_PARAM_TOTAL,   lkey)) return MSG_TEMPLATE_PARAM_TOTAL;
    else if (!strcmp(MSG_TEMPLATE_PARAM_COUNT,   lkey)) return MSG_TEMPLATE_PARAM_COUNT;
    else if (!strcmp(MSG_TEMPLATE_PARAM_DATE,    lkey)) return MSG_TEMPLATE_PARAM_DATE;
    else if (!strcmp(MSG_TEMPLATE_PARAM_ROWS,    lkey)) return MSG_TEMPLATE_PARAM_ROWS;
    else keyIsNotSupported(key); // Forces exception throwing

    return 0;
}

bool MessageFormatter::canAdd(const MissedCallEvent& event)
{
    if (!formatter->isGroupping()) return (events.Count() < Message::maxRowsPerMessage);
    
    const char* fromStr = (event.from.length() > 0) ? event.from.c_str():UNKNOWN_CALLER;
    return ((counters.Exists(fromStr)) ? true:(counters.GetCount() < Message::maxRowsPerMessage));
}
void MessageFormatter::addEvent(const MissedCallEvent& event)
{
    int eventsCount = events.Count();
    if (eventsCount > 0)
    {
        for (int i=0; i<eventsCount; i++) 
            if (event.time < events[i].time) { events.Insert(i, event); break; }
    }
    if (events.Count() == eventsCount) events.Push(event);

    if (formatter->isGroupping())
    {
        const char* fromStr = (event.from.length() > 0) ? event.from.c_str():UNKNOWN_CALLER;
        uint32_t* recPtr = counters.GetPtr(fromStr);
        if (recPtr) (*recPtr)++; else counters.Insert(fromStr, 1);
    }
}
bool MessageFormatter::isLastFromCaller(int index)
{
    if (events.Count() <= 0 || index < 0 || index >= events.Count()) return false;
    else if (index == events.Count()-1) return true;

    const char* checkStr = (events[index].from.length() > 0) ? events[index].from.c_str():UNKNOWN_CALLER;
    for (int i=index+1; i<events.Count(); i++)
    {
        const char* fromStr = (events[i].from.length() > 0) ? events[i].from.c_str():UNKNOWN_CALLER;
        if (!strcmp(fromStr, checkStr)) return false;
    }
    return true;
}
void MessageFormatter::formatMessage(Message& message)
{
    message.message = ""; message.rowsCount = 0; message.eventsCount = 0;
    if (events.Count() <= 0) return;
    message.eventsCount = events.Count(); 

    std::string rows = "";
    ContextEnvironment ctx;
    const std::string unknownCaller = formatter->getUnknownCaller();
    OutputFormatter*  multiFormatter = formatter->getMultiFormatter();
    OutputFormatter*  singleFormatter = formatter->getSingleFormatter();
    OutputFormatter*  messageFormatter = formatter->getMessageFormatter();
    
    for (int i=0; i<events.Count(); i++)
    {
        MissedCallEvent event = events[i];
        const char* fromStr = (event.from.length() > 0) ? event.from.c_str():UNKNOWN_CALLER;
        uint32_t* recPtr = counters.GetPtr(fromStr);
        if (fromStr == UNKNOWN_CALLER || (strcmp(fromStr, UNKNOWN_CALLER) == 0))
            fromStr = unknownCaller.c_str();
        
        if (!formatter->isGroupping() || !recPtr || (recPtr && (*recPtr) <= 1))
        {
            InformGetAdapter adapter(event.to, fromStr, 1, event.time);
            singleFormatter->format(rows, adapter, ctx);
            message.rowsCount++;
        }
        else if (isLastFromCaller(i)) // if event is last from this caller => add it
        {
            InformGetAdapter adapter(event.to, fromStr, (*recPtr), event.time);
            multiFormatter->format(rows, adapter, ctx);
            message.rowsCount++;
        }
    }

    MessageGetAdapter adapter(message.abonent, rows, events.Count());
    messageFormatter->format(message.message, adapter, ctx);
}

}}
