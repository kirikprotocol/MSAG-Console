#ifndef LOG4CPPINIT_H_INCLUDED_C3A87A6B
#define LOG4CPPINIT_H_INCLUDED_C3A87A6B

#include <string>
#include <log4cpp/Category.hh>

namespace log4cpp {
	class Logger
	{
	public:
		/*!
		 * retrieves log4cpp::Category instance for given category name
		 * \param name Category name to retrieve
		 * \return log4cppCategory logger category
		 */
		static Category & getCategory(const std::string &name);

		/*!
		 * Инициализирует log4cpp::Logger по данному файлу конфигурации.
		 * Инициализация происходит только если log4cpp не был проинициализирован до этого.
		 * Если файл конфигурации не найден, или произошла какая-нибудь ошибка при
		 * инициализации, то log4cpp инициализируется параметрами по умолчанию
		 * (файл smsc.log в текущей директории, уровень DEBUG)
		 * \param configFileName имя файла конфигурации log4cpp
		 */
		static void Init(const std::string & configFileName);

		/*!
		 * Деинициализирует log4cpp. После этого его можно снова инициализировать.
		 */
		static void Shutdown();
	protected:
		static bool isInitialized;
	};
}

#endif // ifndef LOG4CPPINIT_H_INCLUDED_C3A87A6B
