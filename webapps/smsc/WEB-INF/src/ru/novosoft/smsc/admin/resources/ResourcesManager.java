package ru.novosoft.smsc.admin.resources;

import java.io.*;
import java.util.List;

/**
 * Created by igork
 * Date: 11.03.2003
 * Time: 17:22:09
 */
public interface ResourcesManager
{
	static final String RESOURCE_FILENAME_PREFIX = "resources_";
	static final String RESOURCE_FILENAME_BODY_PATTERN = ".{2,2}_.{2,2}";
	static final String RESOURCE_FILENAME_SUFFIX_PATTERN = "\\.xml";
	static final String RESOURCE_FILENAME_SUFFIX = ".xml";
	static final String RESOURCE_FILENAME_PATTERN = RESOURCE_FILENAME_PREFIX + RESOURCE_FILENAME_BODY_PATTERN + RESOURCE_FILENAME_SUFFIX_PATTERN;
	static final int RESOURCE_FILENAME_PREFIX_LENGTH = RESOURCE_FILENAME_PREFIX.length();
	static final int RESOURCE_FILENAME_BODY_LENGTH = 5;


	/**
	 * adds new resource file
	 * @param localeName locale name for new resource
	 * @param resourceStream stream with resource data
	 * @throws IOException if can't read or write
	 */
	void add(String localeName, InputStream resourceStream) throws IOException;

	/**
	 * rename existing file to needed resource file
	 * @param localeName
	 * @param resourceFile existing file with resources for this locale
	 * @return true, if locale file sucessfully added
	 */
	boolean add(String localeName, File resourceFile);

	/**
	 * @return список имён локалей (<code>String</code>), для которых есть ресурсы
	 */
	List list();

	/**
	 * Возвращает содержимое ресурса для данной локали
	 * @param localeName имя локали
	 * @return содержимое ресурса для данной локали
	 * @throws FileNotFoundException если ресурс для данной локали не найден
	 */
	BufferedReader getResource(String localeName) throws FileNotFoundException;

	boolean isResourceFileExists(String locale);
}
