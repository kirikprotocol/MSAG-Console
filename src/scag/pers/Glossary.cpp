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
  smsc::core::buffers::File* Glossary::glossFile = NULL;
  Glossary::GlossaryHash Glossary::glossHash;
  Glossary::GlossaryVector Glossary::glossVector;
  int	Glossary::currentIndex = 0;
  bool Glossary::opened = false;
  smsc::core::synchronization::Mutex Glossary::mutex;

};
};
