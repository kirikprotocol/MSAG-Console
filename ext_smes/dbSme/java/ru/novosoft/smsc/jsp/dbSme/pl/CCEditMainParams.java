/*
 * Created by igork
 * Date: 07.10.2002
 * Time: 21:10:36
 */
package ru.novosoft.smsc.jsp.dbSme.pl;

import ru.novosoft.smsc.jsp.dbSme.bl.SMSCInfo;
import ru.novosoft.smsc.jsp.dbSme.bl.ThreadPoolInfo;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

public class CCEditMainParams extends CC
{
	public static final String PARAM_ProtocolId = "pProtocolId";
	public static final String PARAM_SvcType = "pSvcType";
	public static final String PARAM_ThreadPool_Init = "pThreadPoolInit";
	public static final String PARAM_ThreadPool_Max = "pThreadPoolMax";
	public static final String PARAM_SMSC_Host = "pSmscHost";
	public static final String PARAM_SMSC_Port = "pSmscPort";
	public static final String PARAM_SMSC_Sid = "pSmscSid";
	public static final String PARAM_SMSC_Timeout = "pSmscTimeout";
	public static final String PARAM_SMSC_Password = "pSmscPassword";

	public static final String BUTTON_Save = "bSave";
	public static final String BUTTON_Cancel = "bCancel";

	protected int protocolId = -1;
	protected String svcType = null;
	protected int poolInit = -1;
	protected int poolMax = -1;
	protected String smsc_host = null;
	protected int smsc_port = -1;
	protected String smsc_password = null;
	protected String smsc_sid = null;
	protected int smsc_timeout = -1;


	public int process(HttpServletRequest request, HttpServletResponse response)
			  throws Exception
	{
		int result = super.process(request, response);
		if (result == RESULT_Ok || result == RESULT_NotAllParams)
		{
			if (request.getParameter(BUTTON_Save) != null)
			{
				if (result != RESULT_Ok)
					result = RESULT_Error;
				else
					result = processSave();
			}
			else if (request.getParameter(BUTTON_Cancel) != null)
				result = RESULT_Done;
		}
		return result;
	}

	public int processSave()
			  throws Exception
	{
		config.setProtocolId(protocolId);
		config.setSvcType(svcType);
		ThreadPoolInfo pool = config.getThreadPoolInfo();
		pool.setInit(poolInit);
		pool.setMax(poolMax);
		SMSCInfo smsc = config.getSMSCInfo();
		smsc.setHost(smsc_host);
		smsc.setPassword(smsc_password);
		smsc.setPort(smsc_port);
		smsc.setSid(smsc_sid);
		smsc.setTimeout(smsc_timeout);
		config.save();
		return RESULT_Done;
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

	public int loadData(HttpServletRequest request)
	{
		String protocolIdStr = request.getParameter(PARAM_ProtocolId);
		svcType = request.getParameter(PARAM_SvcType);
		String poolInitStr = request.getParameter(PARAM_ThreadPool_Init);
		String poolMaxStr = request.getParameter(PARAM_ThreadPool_Max);
		smsc_host = request.getParameter(PARAM_SMSC_Host);
		String smsc_portStr = request.getParameter(PARAM_SMSC_Port);
		smsc_sid = request.getParameter(PARAM_SMSC_Sid);
		String smsc_timeoutStr = request.getParameter(PARAM_SMSC_Timeout);
		smsc_password = request.getParameter(PARAM_SMSC_Password);
		if (protocolIdStr == null
				  || svcType == null
				  || poolInitStr == null
				  || poolMaxStr == null
				  || smsc_host == null
				  || smsc_portStr == null
				  || smsc_sid == null
				  || smsc_timeoutStr == null
				  || smsc_password == null)
		{
			return RESULT_NotAllParams;
		}

		try
		{
			protocolId = Integer.decode(protocolIdStr).intValue();
			poolInit = Integer.decode(poolInitStr).intValue();
			poolMax = Integer.decode(poolMaxStr).intValue();
			smsc_port = Integer.decode(smsc_portStr).intValue();
			smsc_timeout = Integer.decode(smsc_timeoutStr).intValue();
		}
		catch (NumberFormatException e)
		{
			return RESULT_Error;
		}

		return RESULT_Ok;
	}
}
