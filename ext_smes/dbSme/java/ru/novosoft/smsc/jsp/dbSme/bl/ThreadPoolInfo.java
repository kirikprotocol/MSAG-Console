/*
 * Created by igork
 * Date: 07.10.2002
 * Time: 17:26:34
 */
package ru.novosoft.smsc.jsp.dbSme.bl;

import ru.novosoft.smsc.util.config.Config;

public class ThreadPoolInfo
{
	protected Config config = null;
	protected String prefix = null;

	public ThreadPoolInfo(Config config, String prefix)
	{
		this.config = config;
		this.prefix = prefix;
	}

	public int getInit()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getInt(prefix + ".init");
	}

	public int getMax()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getInt(prefix + ".max");
	}

	public void setInit(int init)
	{
		config.setInt(prefix + ".init", init);
	}

	public void setMax(int max)
	{
		config.setInt(prefix + ".max", max);
	}
}
