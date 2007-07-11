//------------------------------------
//  Glossary.hpp
//  Routman Michael, 2007
//------------------------------------
//
//	���� �������� �������� ������ Glossary.
//

#ifndef ___GLOSSARY_H
#define ___GLOSSARY_H

#include <logger/Logger.h>
#include "core/buffers/File.hpp"

#include "string"
#include "map"
#include "vector"

namespace scag{ namespace pers{
using std::string;
using std::map;
using std::vector;

class Glossary
{
	typedef map<string, int> 		GlossaryMap;
	typedef map<string, int>::iterator	GlossaryMapIterator;
	typedef vector<string>			GlossaryVector;
	
public:
	static const int SUCCESS		= 0;
	static const int OPEN_ERROR		= 1;
	static const int LOAD_ERROR		= 2;
	static const int ALREADY_OPENED		= 3;
	static const int NO_VALUE		= -1;
	static const int NO_KEY			= -2;
	static const int KEY_ALREADY_EXISTS	= -3;	
	static const int GLOSSARY_CLOSED	= -4;

/*	Glossary(const string& glossFileName=""):logger(smsc::logger::Logger::getInstance("Pers.Gloss")), currentIndex(0)
	{
		glossVector.reserve(1024);
		if(glossFileName.length() > 0)
			Init(glossFileName);
	}
	virtual ~Glossary()
	{
		glossMap.erase(glossMap.begin(), glossMap.end());
		glossVector.erase(glossVector.begin(), glossVector.end());
		glossFile.Close();
	}*/
	
	static int Open(const string& glossFileName)
	{
		if(opened) return ALREADY_OPENED;
		logger = smsc::logger::Logger::getInstance("Pers.Gloss");
		currentIndex = 0;
		glossVector.reserve(1024);
		
		try
		{
			if(File::Exists(glossFileName.c_str()))
			{
				glossFile.RWOpen(glossFileName.c_str());
				LoadGlossary();
			}
			else
			{
				glossFile.RWCreate(glossFileName.c_str());
				glossFile.Write("UDEFINED\n", 9);
			}

			glossFile.SetUnbuffered();
		}
		catch(FileException ex)
		{
			smsc_log_debug(logger, "error open or create Glossary File - %s\n", ex.what());
			return OPEN_ERROR;
		}
		opened = true;
		return SUCCESS;
		
	}
	static void Close()
	{
		if(!opened) return;
		glossMap.erase(glossMap.begin(), glossMap.end());
		glossVector.erase(glossVector.begin(), glossVector.end());
		glossFile.Close();
		opened=false;
	}
	
	static int Add(const string& key)
	{
		if(!opened) return GLOSSARY_CLOSED;
		if(NO_VALUE == GetValueByKey(key))
		{
			char ch = 0x0a;
			glossFile.Write(key.c_str(), key.length());
			glossFile.Write(&ch, sizeof(char)); 
			glossMap.insert(GlossaryMap::value_type(key, currentIndex));
			currentIndex++;
			return currentIndex - 1;
		}
		return KEY_ALREADY_EXISTS;
	}
	
	static int GetValueByKey(const string& key)
	{
		if(!opened) return GLOSSARY_CLOSED;
		GlossaryMapIterator It;

		It = glossMap.find(key);
		if(It == glossMap.end())
			return NO_VALUE;
		return It->second;
	}
	static int GetKeyByValue(const int value, string& key)
	{
		if(!opened) return GLOSSARY_CLOSED;
		if(value < glossVector.size())
		{
			key = glossVector[value];
			return SUCCESS;
		}
		return NO_KEY;
	}
	
private:
	static int LoadGlossary(void)
	{
		string	key;
		
		try
		{
			while(glossFile.ReadLine(key))
			{
				glossMap.insert(GlossaryMap::value_type(key, currentIndex));
				glossVector.push_back(key);
				currentIndex++;
			}
			glossFile.SeekEnd(0);
		}
		catch(FileException ex)
		{
			smsc_log_debug(logger, "Load Glossary Error - %s", ex.what());
			return LOAD_ERROR;
		}
		return SUCCESS;
	}
protected:
	static smsc::logger::Logger *logger;
	
	static File		glossFile;
	static GlossaryMap	glossMap;
	static GlossaryVector	glossVector;
	static int		currentIndex;
	static bool		opened;
	
};

};
};

#endif
