#include <string>
#include <iostream>

using namespace std;

/**
 * Основной файл для запуска всех видов тестов.
 * Опции:
 * <ul>
 * <li> -h - получение справки
 * </ul>
 * 
 * @param argc
 * @param argv
 * @return 
 */
int main(int argc, char** argv)
{
	if (argc < 2 || strcmp("-h", argv[1]) == 0)
	{
		cout << "Usage: runtest [options] [test name]" << endl;
		exit(0);
	}

	/*
	cout << "Command line arguments:" << endl;
	for (int i = 0; i < argc; i++)
	{
		cout << argv[i] << endl;
	}
	*/
}
