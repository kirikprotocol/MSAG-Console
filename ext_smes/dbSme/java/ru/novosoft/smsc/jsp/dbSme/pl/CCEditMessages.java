/*
 * Created by igork
 * Date: 08.10.2002
 * Time: 1:49:39
 */
package ru.novosoft.smsc.jsp.dbSme.pl;

import ru.novosoft.smsc.jsp.dbSme.bl.*;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.HashMap;
import java.util.Map;

public class CCEditMessages extends CC
{
	public static final String PARAM_Name = "pName";
	public static final String PARAM_OldName = "pOldName";
	public static final String PARAM_Value = "pValue";
	public static final String PARAM_ProviderName = "pProviderName";
	public static final String PARAM_JobName = "pJobName";

	public static final String BUTTON_Save = "bSave";
	public static final String BUTTON_Cancel = "bCancel";

	protected MessageSet messageSet = null;
	protected Map newMessages = new HashMap();
	protected String providerName = null;
	protected String jobName = null;

	public int process(HttpServletRequest request, HttpServletResponse response) throws Exception
	{
		int result = super.process(request, response);
		if (result == RESULT_Ok)
		{
			if (request.getParameter(BUTTON_Save) != null)
				result = processSave();
			else if (request.getParameter(BUTTON_Cancel) != null)
				result = processCancel();
		}
		return result;
	}

	public int processSave() throws Exception
	{
		for (int i = 0; i < MessageSet.MESSAGES_TYPES.length; i++)
		{
			String name = MessageSet.MESSAGES_TYPES[i];
			String value = (String) newMessages.get(name);
			if (value == null || value.length() == 0)
				messageSet.removeMessage(name);
			else
				messageSet.setMessage(name, value);
		}
		config.save();
		return RESULT_Done;
	}

	public int processCancel()
	{
		return RESULT_Done;
	}

	protected int loadData(HttpServletRequest request)
	{
		providerName = request.getParameter(PARAM_ProviderName);
		jobName = request.getParameter(PARAM_JobName);
		try
		{
			DataProviderInfo provider = null;
			JobInfo job = null;

			if (providerName != null && providerName.length() > 0)
				provider = config.getProvider(providerName);
			if (provider != null && jobName != null && jobName.length() > 0)
				job = provider.getJob(jobName);

			if (job != null)
				messageSet = job.getMessages();
			else if (provider != null)
				messageSet = provider.getMessages();
			else
				messageSet = config.getDBSmeMessages();
		}
		catch (Config.ParamNotFoundException e)
		{
			e.printStackTrace();
			return RESULT_Error;
		}
		catch (Config.WrongParamTypeException e)
		{
			e.printStackTrace();
			return RESULT_Error;
		}
		newMessages.clear();
		for (int i = 0; i < MessageSet.MESSAGES_TYPES.length; i++)
		{
			newMessages.put(MessageSet.MESSAGES_TYPES[i], request.getParameter(PARAM_Name + MessageSet.MESSAGES_TYPES[i]));
		}
		return RESULT_Ok;
	}

	public Map getMessages() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return messageSet.getMessages();
	}

	public String[] getPossibleMessageTypes()
	{
		return MessageSet.MESSAGES_TYPES;
	}

	public String getProviderName()
	{
		return providerName;
	}

	public String getJobName()
	{
		return jobName;
	}

	public int getPossibleStartIndex()
	{
		if (jobName != null && jobName.length() > 0)
			return 2;
		else if (providerName != null && providerName.length() > 0)
			return 1;
		else
			return 0;
	}
}
