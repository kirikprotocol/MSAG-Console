/*
 * Created by igork
 * Date: 07.10.2002
 * Time: 17:29:26
 */
package ru.novosoft.smsc.jsp.dbSme.bl;

import ru.novosoft.smsc.util.config.Config;

public class SMSCInfo
{
	protected Config config = null;
	protected String prefix = null;

	public SMSCInfo(Config config, String prefix)
	{
		this.config = config;
		this.prefix = prefix;
	}

	public String getHost()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefix + ".host");
	}

	public int getPort()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getInt(prefix + ".port");
	}

	public String getSid()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefix + ".sid");
	}

	public int getTimeout()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getInt(prefix + ".timeout");
	}

	public String getPassword()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(prefix + ".password");
	}

	public void setHost(String host)
	{
		config.setString(prefix + ".host", host);
	}

	public void setPort(int port)
	{
		config.setInt(prefix + ".port", port);
	}

	public void setSid(String sid)
	{
		config.setString(prefix + ".sid", sid);
	}

	public void setTimeout(int timeout)
	{
		config.setInt(prefix + ".timeout", timeout);
	}

	public void setPassword(String password)
	{
		config.setString(prefix + ".password", password);
	}
}
