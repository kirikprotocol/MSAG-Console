/*
    test case fot tests 1..6 initialisation RuleEngine and SessionManager
*/
puts("\nTestCase 1: Initiallization\n");
    if(InitReInstance("/export/home/green/install/scag_distr/applet/rules"))
    {
    
	if(InitSessionManagerInstance("bin/rules2",1))
        {
	  /* RuleRun("deliver_sm","81234567",0,1,"89765432",0,1,51,0,1);*/
	}
    }


puts("\nend scripting, enter q to exit.\n");


