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
	 * @return ������ ��� ������� (<code>String</code>), ��� ������� ���� �������
	 */
	List list();

	/**
	 * ���������� ���������� ������� ��� ������ ������
	 * @param localeName ��� ������
	 * @return ���������� ������� ��� ������ ������
	 * @throws FileNotFoundException ���� ������ ��� ������ ������ �� ������
	 */
	BufferedReader getResource(String localeName) throws FileNotFoundException;

	boolean isResourceFileExists(String locale);
}
