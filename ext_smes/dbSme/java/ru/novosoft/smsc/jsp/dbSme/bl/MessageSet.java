/*
 * Created by igork
 * Date: 07.10.2002
 * Time: 17:22:00
 */
package ru.novosoft.smsc.jsp.dbSme.bl;

import ru.novosoft.smsc.util.config.Config;

import java.util.*;

public class MessageSet
{
	public static final String MESSAGES_TYPES[] = {"PROVIDER_NOT_FOUND", "JOB_NOT_FOUND",

																  "DS_FAILURE", "DS_CONNECTION_LOST", "DS_STATEMENT_FAIL",

																  "QUERY_NULL", "INPUT_PARSE", "OUTPUT_FORMAT", "INVALID_CONFIG"};
	public static final Set messagesTypes = new HashSet(Arrays.asList(MESSAGES_TYPES));

	protected Config config = null;
	protected String prefix = null;

	public MessageSet(Config config, String prefix)
	{
		this.config = config;
		this.prefix = prefix;
	}

	public Map getMessages() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		Map messages = new HashMap();
		for (Iterator i = config.getSectionChildParamsNames(prefix + ".MessageSet").iterator(); i.hasNext();)
		{
			String paramName = (String) i.next();
			messages.put(paramName.substring(paramName.lastIndexOf('.') + 1), config.getString(paramName));
		}
		return messages;
	}

	public void setMessage(String name, String value)
	{
		String pp = prefix + ".MessageSet";
		config.setString(pp + '.' + name, value);
	}

	public void addMessage(String name, String value)
	{
		this.setMessage(name, value);
	}

	public void removeMessage(String name)
	{
		String pp = prefix + ".MessageSet";
		config.removeParam(pp + '.' + name);
	}
}
