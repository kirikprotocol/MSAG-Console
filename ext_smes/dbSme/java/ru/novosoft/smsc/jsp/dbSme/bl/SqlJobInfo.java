/*
 * Created by igork
 * Date: 07.10.2002
 * Time: 17:50:31
 */
package ru.novosoft.smsc.jsp.dbSme.bl;

import ru.novosoft.smsc.util.config.Config;

public class SqlJobInfo extends JobInfo
{
	public static final String JOB_TYPE_Sql = "sql-job";

	public SqlJobInfo(Config config, String prefix)
	{
		super(config, prefix);
	}

	public boolean isQuery() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getBool(prefixName + ".query");
	}

	public void setQuery(boolean query)
	{
		config.setBool(prefixName + ".query", query);
	}

}
