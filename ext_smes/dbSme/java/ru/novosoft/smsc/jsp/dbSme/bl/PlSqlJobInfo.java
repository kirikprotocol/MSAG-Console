package ru.novosoft.smsc.jsp.dbSme.bl;

import ru.novosoft.smsc.util.config.Config;

/**
 * Created by igork
 * Date: 04.04.2003
 * Time: 19:19:24
 */
public class PlSqlJobInfo extends JobInfo
{
	public static final String JOB_TYPE_PlSql = "pl/sql-job";

	public PlSqlJobInfo(Config config, String prefixName)
	{
		super(config, prefixName);
	}

	public boolean isFunction() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getBool(prefixName + ".function");
	}

	public void setFunction(boolean function)
	{
		config.setBool(prefixName + ".function", function);
	}

	public boolean isCommit() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getBool(prefixName + ".commit");
	}

	public void setCommit(boolean commit)
	{
		config.setBool(prefixName + ".commit", commit);
	}
}
