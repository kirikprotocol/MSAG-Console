/*
 * Created by igork
 * Date: 07.10.2002
 * Time: 17:50:31
 */
package ru.novosoft.smsc.jsp.dbSme.bl;

import ru.novosoft.smsc.util.config.Config;

public class JobInfo
{
	protected Config config;
	protected String prefixName;

	public JobInfo(Config config, String prefix)
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		this.config = config;
		this.prefixName = prefix;
	}

	public String getName()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefixName + ".name");
	}

	public void setName(String name)
	{
		config.setString(prefixName + ".name", name);
	}

	public String getType()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefixName + ".type");
	}

	public void setType(String type)
	{
		config.setString(prefixName + ".type", type);
	}

	public boolean isQuery()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getBool(prefixName + ".query");
	}

	public void setQuery(boolean query)
	{
		config.setBool(prefixName + ".query", query);
	}

	public String getSql()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefixName + ".sql");
	}

	public void setSql(String sql)
	{
		config.setString(prefixName + ".sql", sql);
	}

	public String getInput()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefixName + ".input");
	}

	public void setInput(String input)
	{
		config.setString(prefixName + ".input", input);
	}

	public String getOutput()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefixName + ".output");
	}

	public void setOutput(String output)
	{
		config.setString(prefixName + ".output", output);
	}

	public MessageSet getMessages()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return new MessageSet(config, prefixName);
	}
}
