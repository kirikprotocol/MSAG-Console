/*
 * Created by igork
 * Date: 07.10.2002
 * Time: 23:06:06
 */
package ru.novosoft.smsc.jsp.dbSme.pl;

import ru.novosoft.smsc.jsp.dbSme.bl.DataSourceDriverInfo;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Iterator;
import java.util.List;

public class CCEditDataSources extends CC
{
	public static final String PARAM_Type = "pType";
	public static final String PARAM_OldName = "pOldName";
	public static final String PARAM_Loadup = "pLoadup";

	public static final String BUTTON_Save = "bSave";
	public static final String BUTTON_Remove = "bRemove";
	public static final String BUTTON_Add = "bAdd";

	protected String type = null;
	protected String name = null;
	protected String oldname = null;
	protected String loadup = null;

	public int process(HttpServletRequest request, HttpServletResponse response)
			  throws Exception
	{
		int result = super.process(request, response);
		if (result == RESULT_Ok)
		{
			if (request.getParameter(BUTTON_Add) != null)
				result = processAdd();
			else if (request.getParameter(BUTTON_Save) != null)
				result = processSave();
			else if (request.getParameter(BUTTON_Remove) != null)
				result = processRemove();
		}
		return result;
	}

	protected DataSourceDriverInfo findDataSource(String datasourceName)
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		List sources = config.getDataSourceDrivers();
		for (Iterator i = sources.iterator(); i.hasNext();)
		{
			DataSourceDriverInfo info = (DataSourceDriverInfo) i.next();
			if (info.getName().equals(datasourceName))
				return info;
		}
		return null;
	}

	protected int processAdd()
			  throws Exception
	{
		if (findDataSource(name) != null)
			return RESULT_Error;
		config.addDataSourceDriver(name, type, loadup);
		config.save();
		return RESULT_Done;
	}

	protected int processSave()
			  throws Exception
	{
		if (oldname == null || oldname.length() == 0)
			return RESULT_NotAllParams;
		System.out.println("CCEditDataSources.processSave");
		System.out.println("oldname = " + oldname);
		DataSourceDriverInfo info = findDataSource(oldname);
		if (info == null)
			return RESULT_Error;
		System.out.println("CCEditDataSources.processSave 2");
		if (!oldname.equals(name))
		{
			config.removeDataSourceDriver(oldname);
			config.addDataSourceDriver(name, type, loadup);
		}
		else
		{
			info.setLoadup(loadup);
			info.setType(type);
		}
		config.save();
		return RESULT_Done;
	}

	protected int processRemove()
			  throws Exception
	{
		if (oldname == null || oldname.length() == 0)
			return RESULT_NotAllParams;
		System.out.println("CCEditDataSources.processRemove");
		System.out.println("oldname = " + oldname);
		DataSourceDriverInfo info = findDataSource(oldname);
		if (info == null)
			return RESULT_Error;
		config.removeDataSourceDriver(oldname);
		config.save();
		return RESULT_Done;
	}

	protected int loadData(HttpServletRequest request)
	{
		type = request.getParameter(PARAM_Type);
		name = type;
		oldname = request.getParameter(PARAM_OldName);
		loadup = request.getParameter(PARAM_Loadup);
		return RESULT_Ok;
/*
		if (type != null && name != null && loadup != null && name.length() > 0)
			return RESULT_Ok;
		else
			return RESULT_NotAllParams;
*/
	}

	public List getDataSourceDrivers()
			  throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return config.getDataSourceDrivers();
	}
}
