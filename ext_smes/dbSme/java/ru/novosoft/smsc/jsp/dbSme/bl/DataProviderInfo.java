/*
 * Created by igork
 * Date: 07.10.2002
 * Time: 17:20:37
 */
package ru.novosoft.smsc.jsp.dbSme.bl;

import ru.novosoft.smsc.util.config.Config;

import java.util.*;

public class DataProviderInfo
{
	protected Config config = null;
	protected String prefix = null;
	protected String name;

	public DataProviderInfo(Config config, String prefix, String name) throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		this.config = config;
		this.prefix = prefix;
		this.name = name;
	}

	public String getNamePrefix()
	{
		return prefix + '.' + name;
	}

	protected String getPrefixJobs()
	{
		return prefix + '.' + name + ".Jobs";
	}

	public String getName()
	{
		return name;
	}

	public void setName(String name)
	{
		this.name = name;
	}

	public String getAddress() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(getNamePrefix() + ".address");
	}

	public void setAddress(String address)
	{
		config.setString(getNamePrefix() + ".address", address);
	}

	public DataSourceInfo getDataSourceInfo() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return new DataSourceInfo(config, getNamePrefix() + ".DataSource");
	}

	public List getJobs() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		List result = new LinkedList();
		Collection jobnames = config.getSectionChildSectionNames(getPrefixJobs());
		for (Iterator i = jobnames.iterator(); i.hasNext();)
		{
			String jobName = (String) i.next();
			String type = getJobType(jobName);
			if (type.equals(SqlJobInfo.JOB_TYPE_Sql))
				result.add(new SqlJobInfo(config, jobName));
			else if (type.equals(PlSqlJobInfo.JOB_TYPE_PlSql))
				result.add(new PlSqlJobInfo(config, jobName));
		}
		return result;
	}

	public MessageSet getMessages() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return new MessageSet(config, getNamePrefix());
	}

	public JobInfo getJob(String jobName) throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		Collection jobnames = config.getSectionChildSectionNames(getPrefixJobs());
		for (Iterator i = jobnames.iterator(); i.hasNext();)
		{
			String fullName = (String) i.next();
			String name = config.getString(fullName + ".name");
			if (name.equals(jobName))
			{
				String jobType = getJobType(fullName);
				if (SqlJobInfo.JOB_TYPE_Sql.equals(jobType))
				{
					return new SqlJobInfo(config, fullName);
				}
				else if (PlSqlJobInfo.JOB_TYPE_PlSql.equals(jobType))
				{
					return new PlSqlJobInfo(config, fullName);
				}
			}
		}
		return null;
	}

	private String getJobType(final String jobPrefixName) throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(jobPrefixName + ".type");
	}

	public void createSqlJob(String jobName, boolean jobQuery, String address, String alias, String jobSql, String jobInput, String jobOutput)
	{
		String prefix = getPrefixJobs() + '.' + jobName;
		config.setString(prefix + ".name", jobName);
		config.setString(prefix + ".type", SqlJobInfo.JOB_TYPE_Sql);
		config.setBool(prefix + ".query", jobQuery);
		config.setString(prefix + ".address", address);
		config.setString(prefix + ".alias", alias);
		config.setString(prefix + ".sql", jobSql);
		config.setString(prefix + ".input", jobInput);
		config.setString(prefix + ".output", jobOutput);
	}

	public void createPlSqlJob(String jobName, boolean commit, boolean function, String jobSql, String jobInput, String jobOutput)
	{
		String prefix = getPrefixJobs() + '.' + jobName;
		config.setString(prefix + ".name", jobName);
		config.setString(prefix + ".type", PlSqlJobInfo.JOB_TYPE_PlSql);
		config.setBool(prefix + ".commit", commit);
		config.setBool(prefix + ".function", function);
		config.setString(prefix + ".sql", jobSql);
		config.setString(prefix + ".input", jobInput);
		config.setString(prefix + ".output", jobOutput);
	}

	public void deleteJob(String jobName) throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		String prefix = getPrefixJobs() + '.' + jobName;
		config.removeParam(prefix + ".name");
		config.removeParam(prefix + ".type");
		config.removeParam(prefix + ".query");
		config.removeParam(prefix + ".address");
		config.removeParam(prefix + ".alias");
		config.removeParam(prefix + ".sql");
		config.removeParam(prefix + ".input");
		config.removeParam(prefix + ".output");
		config.removeParam(prefix + ".commit");
		config.removeParam(prefix + ".function");
	}
}
