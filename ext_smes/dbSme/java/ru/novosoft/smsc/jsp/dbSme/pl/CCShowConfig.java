/*
 * Created by igork
 * Date: 04.10.2002
 * Time: 19:03:13
 */
package ru.novosoft.smsc.jsp.dbSme.pl;

import ru.novosoft.smsc.jsp.dbSme.bl.MessageSet;
import ru.novosoft.smsc.jsp.dbSme.bl.SMSCInfo;
import ru.novosoft.smsc.jsp.dbSme.bl.ThreadPoolInfo;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

public class CCShowConfig extends CC
{
	public List getDataSourceDrivers()
			  throws Exception
	{
		return config.getDataSourceDrivers();
	}

	public MessageSet getDbSmeMessages()
			  throws Exception
	{
		return config.getDBSmeMessages();
	}

	public String getSvcType()
			  throws Exception
	{
		return config.getSvcType();
	}

	public int getProtocolId()
			  throws Exception
	{
		return config.getProtocolId();
	}

	public ThreadPoolInfo getThreadPoolInfo()
			  throws Exception
	{
		return config.getThreadPoolInfo();
	}

	public SMSCInfo getSMSCInfo()
			  throws Exception
	{
		return config.getSMSCInfo();
	}

	public List getProviders()
			  throws Exception
	{
		return config.getProviders();
	}

	public int loadData(HttpServletRequest request)
	{
		return RESULT_Ok;
	}

	public String[] getPossibleMessageTypes()
	{
		return MessageSet.MESSAGES_TYPES;
	}
}
