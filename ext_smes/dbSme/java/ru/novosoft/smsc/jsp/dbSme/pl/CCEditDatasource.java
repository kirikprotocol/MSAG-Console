/*
 * Created by igork
 * Date: 09.10.2002
 * Time: 1:19:31
 */
package ru.novosoft.smsc.jsp.dbSme.pl;

import ru.novosoft.smsc.jsp.dbSme.bl.DataProviderInfo;
import ru.novosoft.smsc.jsp.dbSme.bl.DataSourceInfo;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

public class CCEditDatasource extends CC
{
	public static final String PARAM_ProviderName = "pProviderName";
	public static final String PARAM_OldProviderName = "pOldProviderName";
	public static final String PARAM_Address = "pAddress";

	public static final String PARAM_Type = "pType";
	public static final String PARAM_Connections = "pConnections";
	public static final String PARAM_DbInstance = "pDbInstance";
	public static final String PARAM_DbUserName = "pDbUserName";
	public static final String PARAM_DbUserPassword = "pDbUserPassword";
	public static final String PARAM_Creating = "pCreating";

	public static final String BUTTON_Save = "bSave";
	public static final String BUTTON_Cancel = "bCancel";
	public static final String BUTTON_Create = "bCreate";

	protected String providerName = null;
	protected String oldProviderName = null;
	protected String address = null;
	protected DataProviderInfo provider = null;
	protected DataSourceInfo dataSource = null;

	protected String type = null;
	protected int connections = -1;
	protected String dbInstance = null;
	protected String dbUserName = null;
	protected String dbUserPassword = null;
	private boolean buttonCreate = false;
	private boolean buttonSave = false;
	private boolean buttonCancel = false;
	private boolean creating = false;


	public int process(HttpServletRequest request, HttpServletResponse response)
			  throws Exception
	{
		int result = super.process(request, response);
		if (result == RESULT_Ok)
		{
			if (buttonCreate)
				result = processCreate();
			else if (buttonSave)
				result = processSave();
			else if (buttonCancel)
				result = processCancel();
		}
		return result;
	}

	protected int processCreate()
			  throws Exception
	{
		creating = true;
		return RESULT_Ok;
	}

	protected int processSave()
			  throws Exception
	{
		if (isCreating())
		{
			if (config.getProvider(providerName) != null)
				return RESULT_Error;
			config.addProvider(providerName, address, type, connections, dbInstance, dbUserName, dbUserPassword);
		}
		else
		{
			provider.setAddress(address);
			dataSource.setType(type);
			dataSource.setConnections(connections);
			dataSource.setDbInstance(dbInstance);
			dataSource.setDbUserName(dbUserName);
			dataSource.setDbUserPassword(dbUserPassword);
			if (!oldProviderName.equals(providerName))
				config.renameProvider(oldProviderName, providerName);
		}
		config.save();
		return RESULT_Done;
	}

	protected int processCancel()
	{
		return RESULT_Done;
	}

	protected int loadData(HttpServletRequest request)
	{
		buttonCreate = request.getParameter(BUTTON_Create) != null;
		buttonSave = request.getParameter(BUTTON_Save) != null;
		buttonCancel = request.getParameter(BUTTON_Cancel) != null;
		if (buttonCreate)
		{
			providerName = oldProviderName = address = type = dbInstance = dbUserName = dbUserPassword = "";
			connections = 1;
		}
		else
		{
			creating = Boolean.valueOf(request.getParameter(PARAM_Creating)).booleanValue();
			try
			{
				oldProviderName = request.getParameter(PARAM_OldProviderName);
				providerName = request.getParameter(PARAM_ProviderName);
				if (providerName == null)
					return RESULT_NotAllParams;
				if (oldProviderName == null)
					oldProviderName = providerName;

				address = request.getParameter(PARAM_Address);
				type = request.getParameter(PARAM_Type);
				String connectionsStr = request.getParameter(PARAM_Connections);
				dbInstance = request.getParameter(PARAM_DbInstance);
				dbUserName = request.getParameter(PARAM_DbUserName);
				dbUserPassword = request.getParameter(PARAM_DbUserPassword);

				if (!creating)
				{
					provider = config.getProvider(oldProviderName);
					if (provider == null)
						return RESULT_Error;
					dataSource = provider.getDataSourceInfo();
					if (dataSource == null)
						return RESULT_Error;

					if (connectionsStr == null)
						connections = dataSource.getConnections();
					else
						connections = Integer.decode(connectionsStr).intValue();

					if (address == null || type == null || dbInstance == null || dbUserName == null || dbUserPassword == null)
					{
						address = provider.getAddress();
						type = dataSource.getType();
						dbInstance = dataSource.getDbInstance();
						dbUserName = dataSource.getDbUserName();
						dbUserPassword = dataSource.getDbUserPassword();
					}
				}
			}
			catch (Config.ParamNotFoundException e)
			{
				return RESULT_Error;
			}
			catch (Config.WrongParamTypeException e)
			{
				return RESULT_Error;
			}
		}

		return RESULT_Ok;
	}

	public String getProviderName()
	{
		return providerName;
	}

	public String getAddress()
	{
		return address;
	}

	public String getType()
	{
		return type;
	}

	public int getConnections()
	{
		return connections;
	}

	public String getDbInstance()
	{
		return dbInstance;
	}

	public String getDbUserName()
	{
		return dbUserName;
	}

	public String getDbUserPassword()
	{
		return dbUserPassword;
	}

	public boolean isCreating()
	{
		return creating;
	}
}
