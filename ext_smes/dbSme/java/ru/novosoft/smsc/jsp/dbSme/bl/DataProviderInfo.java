/*
 * Created by igork
 * Date: 07.10.2002
 * Time: 17:20:37
 */
package ru.novosoft.smsc.jsp.dbSme.bl;

import ru.novosoft.smsc.util.config.Config;

import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

public class DataProviderInfo
{
	protected Config config = null;
	protected String prefix = null;
	protected String name;

	public DataProviderInfo(Config config, String prefix, String name)
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
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

	public String getAddress()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getString(getNamePrefix() + ".address");
	}

	public void setAddress(String address)
	{
		config.setString(getNamePrefix() + ".address", address);
	}

	public DataSourceInfo getDataSourceInfo()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return new DataSourceInfo(config, getNamePrefix() + ".DataSource");
	}

	public List getJobs()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		List result = new LinkedList();
		Collection jobnames = config.getSectionChildSectionNames(getPrefixJobs());
		for (Iterator i = jobnames.iterator(); i.hasNext();)
		{
			String jobName = (String) i.next();
			result.add(new JobInfo(config, jobName));
		}
		return result;
	}

	public MessageSet getMessages()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return new MessageSet(config, getNamePrefix());
	}

	public JobInfo getJob(String jobName)
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		Collection jobnames = config.getSectionChildSectionNames(getPrefixJobs());
		for (Iterator i = jobnames.iterator(); i.hasNext();)
		{
			String fullName = (String) i.next();
			String name = config.getString(fullName + ".name");
			if (name.equals(jobName))
				return new JobInfo(config, fullName);
		}
		return null;
	}

	public void createJob(String jobName, String jobType, boolean jobQuery, String jobSql, String jobInput, String jobOutput)
	{
		String prefix = getPrefixJobs() + '.' + jobName;
		config.setString(prefix + ".name", jobName);
		config.setString(prefix + ".type", jobType);
		config.setBool(prefix + ".query", jobQuery);
		config.setString(prefix + ".sql", jobSql);
		config.setString(prefix + ".input", jobInput);
		config.setString(prefix + ".output", jobOutput);
	}

	public void deleteJob(String jobName)
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		JobInfo job = getJob(jobName);
		config.removeParam(job.prefixName + ".name");
		config.removeParam(job.prefixName + ".type");
		config.removeParam(job.prefixName + ".query");
		config.removeParam(job.prefixName + ".sql");
		config.removeParam(job.prefixName + ".input");
		config.removeParam(job.prefixName + ".output");
	}
}
