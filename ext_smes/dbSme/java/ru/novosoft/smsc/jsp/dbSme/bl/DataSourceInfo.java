/*
 * Created by igork
 * Date: 07.10.2002
 * Time: 17:43:44
 */
package ru.novosoft.smsc.jsp.dbSme.bl;

import ru.novosoft.smsc.util.config.Config;

public class DataSourceInfo
{
	protected Config config = null;
	protected String prefix = null;

	public DataSourceInfo(Config config, String prefix)
	{
		this.config = config;
		this.prefix = prefix;
	}

	public String getType()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefix + ".type");
	}

	public void setType(String type)
	{
		config.setString(prefix + ".type", type);
	}

	public int getConnections()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getInt(prefix + ".connections");
	}

	public void setConnections(int connections)
	{
		config.setInt(prefix + ".connections", connections);
	}

	public String getDbInstance()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefix + ".dbInstance");
	}

	public void setDbInstance(String dbInstance)
	{
		config.setString(prefix + ".dbInstance", dbInstance);
	}

	public String getDbUserName()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefix + ".dbUserName");
	}

	public void setDbUserName(String dbUserName)
	{
		config.setString(prefix + ".dbUserName", dbUserName);
	}

	public String getDbUserPassword()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefix + ".dbUserPassword");
	}

	public void setDbUserPassword(String dbUserPassword)
	{
		config.setString(prefix + ".dbUserPassword", dbUserPassword);
	}
}
