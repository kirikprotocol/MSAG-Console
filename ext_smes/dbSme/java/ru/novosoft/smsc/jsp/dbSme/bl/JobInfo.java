package ru.novosoft.smsc.jsp.dbSme.bl;

import ru.novosoft.smsc.util.config.Config;

/**
 * Created by igork
 * Date: 04.04.2003
 * Time: 19:03:05
 */
public abstract class JobInfo
{
	protected Config config;
	protected String prefixName;

	protected JobInfo(Config config, String prefixName)
	{
		this.config = config;
		this.prefixName = prefixName;
	}

	public String getName() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefixName + ".name");
	}

	public void setName(String name)
	{
		config.setString(prefixName + ".name", name);
	}

	public String getType() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefixName + ".type");
	}

	public void setType(String type)
	{
		config.setString(prefixName + ".type", type);
	}

	public String getSql() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefixName + ".sql");
	}

	public void setSql(String sql)
	{
		config.setString(prefixName + ".sql", sql);
	}

	public String getInput() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefixName + ".input");
	}

	public void setInput(String input)
	{
		config.setString(prefixName + ".input", input);
	}

	public String getOutput() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefixName + ".output");
	}

	public void setOutput(String output)
	{
		config.setString(prefixName + ".output", output);
	}

	public MessageSet getMessages()
	{
		return new MessageSet(config, prefixName);
	}

	public String getAddress() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefixName + ".address");
	}

	public void setAddress(String address)
	{
		config.setString(prefixName + ".address", address);
	}

	public String getAlias() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefixName + ".alias");
	}

	public void setAlias(String alias)
	{
		config.setString(prefixName + ".alias", alias);
	}

	public int getTimeout() throws Config.WrongParamTypeException
	{
		try
		{
			return config.getInt(prefixName + ".timeout");
		}
		catch (Config.ParamNotFoundException e)
		{
			return 0;
		}
	}

	public void setTimeout(int timeout)
	{
		config.setInt(prefixName + ".timeout", timeout);
	}
}
