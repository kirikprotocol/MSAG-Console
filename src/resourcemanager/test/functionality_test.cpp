#include <resourcemanager/ResourceManager.hpp>
#include <iostream>

using namespace smsc::resourcemanager;

int main(int argc, char *argv[])
{
	ResourceManager manager = ResourceManager::getInstance();
	manager.dump(std::cout);
}
