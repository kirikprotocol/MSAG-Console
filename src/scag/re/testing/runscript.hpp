extern bool stopProcess;
using namespace smsc::core::threads;

class CScriptRunner:public Thread
{

public:
 std::string scriptfilename;

public:


/**/
 int initre()
 {

    int errorCount;
    int errorCode;
    SmppCommand command;
    engine = 0;

    smsc::logger::Logger::Init();
    logger = smsc::logger::Logger::getInstance("scag.ruleengine.testing");

    if (!logger)
    {
        printf("error:can`t create logger\n");
        fflush(stdout);
        return 0;
    }
    engine = new RuleEngine("./rules");


    return 1;
 }
/**/

 int Execute()
 {
  if(!initre())
        return 0;

  runScript();
  delete engine;
  smsc::logger::Logger::Shutdown();
  return 1;
 };