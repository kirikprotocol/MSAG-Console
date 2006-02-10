/*
    test case fot tests 1..6 initialisation RuleEngine and SessionManager
*/
puts("\nTestCase 1: Initiallization\n");

if(InitReInstance("./rules"))
{

      if(InitSessionManagerInstance("./store",1))
      {
	  RuleRun("deliver_sm","81234567",0,1,"89765431",0,1,51,0,1);
	  Sleep(5);
	  RuleRun("submit_sm","82234567",0,1,"89765432",0,1,51,0,1);
	  
      }
      else
      {
      puts("Error init instance SManager");
      }
}
else
{
puts("Error init instance REngine");
}


puts("\nend scripting, enter q to exit.\n");


