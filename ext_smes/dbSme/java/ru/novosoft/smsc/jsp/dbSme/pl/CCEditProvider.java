/*
 * Created by igork
 * Date: 08.10.2002
 * Time: 23:28:50
 */
package ru.novosoft.smsc.jsp.dbSme.pl;

import ru.novosoft.smsc.jsp.dbSme.bl.DataProviderInfo;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Iterator;
import java.util.List;
import java.net.URLEncoder;

public class CCEditProvider extends CC
{
	public static final String PARAM_Name = "pName";

	protected DataProviderInfo provider = null;

	public int process(HttpServletRequest request, HttpServletResponse response)
			  throws Exception
	{
		return super.process(request, response);
	}

	protected int loadData(HttpServletRequest request)
	{
		String name = request.getParameter(PARAM_Name);
		if (name == null || name.length() == 0)
			return RESULT_NotAllParams;

		try
		{
			provider = config.getProvider(name);
		}
		catch (Config.ParamNotFoundException e)
		{
			return RESULT_Error;
		}
		catch (Config.WrongParamTypeException e)
		{
			return RESULT_Error;
		}

		return RESULT_Ok;
	}

	public DataProviderInfo getProvider()
	{
		return provider;
	}
}
