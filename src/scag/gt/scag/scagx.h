#ifndef SCAG_X_H
#define SCAG_X_H
namespace scag{
class Scag
{
public:
  Scag():stopFlag(false)
  {
  
  };
  
  ~Scag();
  
  void init(){};
  
  void run()
  {
   while(!stopFlag)
   {
      sleep(1);
   }

  };
  void stop(){stopFlag=true;}
protected:
 bool stopFlag;

};

}//scag
#endif
