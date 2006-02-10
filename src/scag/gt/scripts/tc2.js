/*
    test case fot tests 1..6 initialisation RuleEngine and SessionManager
*/
puts("\nTestCase 1: Initiallization\n");

if(InitReInstance("./rules"))
{

      if(InitSessionManagerInstance("./store",1))
      {
        var k=0;
      
         RuleRun("deliver_sm","81234567",0,1,"87654321",0,1,51,0,1);
	 Sleep(5);

	 RuleRun("deliver_sm","71234577",0,1,"77754777",0,1,51,0,1);
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


