#include <resourcemanager/ResourceManager.hpp>
#include <iostream>

using namespace smsc::resourcemanager;

int main(int argc, char *argv[])
{
	ResourceManager::init("en_en, ru_ru, blah_blah", "blah_blah");
	const ResourceManager* manager = ResourceManager::getInstance();
	manager->dump(std::cout);
}
