//------------------------------------
//  Glossary.cpp
//  Routman Michael, 2007
//------------------------------------
//
//	���� �������� �������� ������ Glossary.
//


#include "Glossary.h"

namespace scag{ namespace pers{
	
smsc::logger::Logger* Glossary::logger = 0;

File*				Glossary::glossFile = NULL;
Glossary::GlossaryMap		Glossary::glossMap;
Glossary::GlossaryVector	Glossary::glossVector;
int				Glossary::currentIndex = 0;
bool				Glossary::opened = false;
Mutex				Glossary::mutex;
};
};