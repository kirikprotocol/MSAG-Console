/*
 * Created by igork
 * Date: 04.10.2002
 * Time: 22:35:25
 */
package ru.novosoft.smsc.jsp.dbSme.bl;

import ru.novosoft.smsc.util.config.Config;

public class DataSourceDriverInfo
{
	protected Config config = null;
	protected String name = null;
	protected String prefixName = null;

	public DataSourceDriverInfo(Config config, String prefix, String name)
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		this.config = config;
		this.name = name;
		prefixName = prefix + '.' + name;
	}

	public String getName()
	{
		return name;
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

	public String getLoadup()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefixName + ".loadup");
	}

	public void setLoadup(String loadup)
	{
		config.setString(prefixName + ".loadup", loadup);
	}
}
