/*
 * Created by igork
 * Date: 09.10.2002
 * Time: 16:52:42
 */
package ru.novosoft.smsc.jsp.dbSme.pl;

import ru.novosoft.smsc.jsp.dbSme.bl.*;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

public class CCEditJob extends CC
{
	public static final String DEFAULT_Type = SqlJobInfo.JOB_TYPE_Sql;

	public static final String PARAM_ProviderName = "pProviderName";

	public static final String PARAM_Name = "pName";
	public static final String PARAM_OldName = "pOldName";
	public static final String PARAM_Type = "pType";
	public static final String PARAM_Query = "pQuery";
	public static final String PARAM_Sql = "pSql";
	public static final String PARAM_Input = "pInput";
	public static final String PARAM_Output = "pOutput";
	public static final String PARAM_IsCreating = "pIsCreating";
	public static final String PARAM_Address = "pAddress";
	public static final String PARAM_Alias = "pAlias";
	public static final String PARAM_Commit = "pCommit";
	public static final String PARAM_Function = "pFunction";

	public static final String BUTTON_Save = "bSave";
	public static final String BUTTON_Cancel = "bCancel";
	public static final String BUTTON_CreateNewJob = "bCreateNewJob";
	public static final String BUTTON_DeleteJob = "bDeleteJob";

	protected String providerName = null;
	protected DataProviderInfo provider = null;
	protected JobInfo job = null;

	protected String name = null;
	protected String oldName = null;
	protected String type = null;
	protected boolean query = false;
	protected String address = null;
	protected String alias = null;
	protected String sql = null;
	protected String input = null;
	protected String output = null;
	protected boolean commit = false;
	protected boolean function = false;

	protected boolean isSaveButton = false;
	protected boolean isCancelButton = false;
	protected boolean isCreating = false;
	protected boolean isCreateNewJobButton = false;
	protected boolean isDeleteJobButton = false;

	public int process(HttpServletRequest request, HttpServletResponse response) throws Exception
	{
		int result = super.process(request, response);
		if (result == RESULT_Ok)
		{
			if (isSaveButton)
				result = processSave();
			else if (isCancelButton)
				result = processCancel();
			else if (isCreateNewJobButton)
				result = processCreate();
			else if (isDeleteJobButton)
				result = processDelete();
		}
		return result;
	}

	protected int processSave() throws Exception
	{
		if (isCreating)
		{
			if (provider.getJob(name) != null)
				return RESULT_Error;
			if (type.equals(SqlJobInfo.JOB_TYPE_Sql))
				provider.createSqlJob(name, query, address, alias, sql, input, output);
			else if (type.equals(PlSqlJobInfo.JOB_TYPE_PlSql))
				provider.createPlSqlJob(name, commit, function, sql, input, output);
			else
				return RESULT_Error;
		}
		else
		{
			System.out.println("name = " + name);
			job.setName(name);
			job.setType(type);
			job.setSql(sql);
			job.setInput(input);
			job.setOutput(output);
			if (type.equals(SqlJobInfo.JOB_TYPE_Sql))
			{
				((SqlJobInfo) job).setQuery(query);
				((SqlJobInfo) job).setAddress(address);
				((SqlJobInfo) job).setAlias(alias);
			}
			else if (type.equals(PlSqlJobInfo.JOB_TYPE_PlSql))
			{
				((PlSqlJobInfo) job).setCommit(commit);
				((PlSqlJobInfo) job).setFunction(function);
			}
			else
				return RESULT_Error;
		}
		config.save();
		return RESULT_Done;
	}

	protected int processCancel()
	{
		return RESULT_Done;
	}

	protected int processCreate()
	{
		System.out.println("CCEditJob.processCreate");
		name = "new job";
		type = DEFAULT_Type;
		query = false;
		sql = "";
		address = "";
		alias = "";
		input = "";
		output = "";
		commit = false;
		function = false;
		isCreating = true;
		return RESULT_Ok;
	}

	protected int processDelete() throws Exception
	{
		provider.deleteJob(oldName);
		config.save();
		return RESULT_Done;
	}

	protected int loadData(HttpServletRequest request)
	{
		isSaveButton = request.getParameter(BUTTON_Save) != null;
		isCancelButton = request.getParameter(BUTTON_Cancel) != null;
		isCreating = request.getParameter(PARAM_IsCreating) != null;
		isCreateNewJobButton = request.getParameter(BUTTON_CreateNewJob) != null;
		isDeleteJobButton = request.getParameter(BUTTON_DeleteJob) != null;

		try
		{
			System.out.println("CCEditJob.loadData 1");
			providerName = request.getParameter(PARAM_ProviderName);
			if (providerName == null)
				return RESULT_NotAllParams;

			System.out.println("CCEditJob.loadData 2");
			provider = config.getProvider(providerName);
			if (provider == null)
				return RESULT_Error;

			System.out.println("CCEditJob.loadData 3");
			if (!isCreateNewJobButton)
			{
				System.out.println("CCEditJob.loadData 4");
				oldName = request.getParameter(PARAM_OldName);
				if (oldName == null)
					oldName = request.getParameter(PARAM_Name);

				if (oldName == null && !isCreating)
					return RESULT_NotAllParams;

				if (oldName != null)
					job = provider.getJob(oldName);
				else
					job = null;

				if (job == null && !isCreating)
					return RESULT_Error;

				name = request.getParameter(PARAM_Name);
				type = request.getParameter(PARAM_Type);
				address = request.getParameter(PARAM_Address);
				alias = request.getParameter(PARAM_Alias);
				String queryStr = request.getParameter(PARAM_Query);
				String functionStr = request.getParameter(PARAM_Function);
				String commitStr = request.getParameter(PARAM_Commit);
				sql = request.getParameter(PARAM_Sql);
				input = request.getParameter(PARAM_Input);
				output = request.getParameter(PARAM_Output);

				if (address == null)
					address = "";
				if (alias == null)
					alias = "";

				if ((name == null || type == null || sql == null || input == null || output == null) && !isCreating)
				{
					name = job.getName();
					type = job.getType();
					sql = job.getSql();
					input = job.getInput();
					output = job.getOutput();
					if (type.equals(SqlJobInfo.JOB_TYPE_Sql))
					{
						address = ((SqlJobInfo) job).getAddress();
						alias = ((SqlJobInfo) job).getAlias();
						query = ((SqlJobInfo) job).isQuery();
					}
					else if (type.equals(PlSqlJobInfo.JOB_TYPE_PlSql))
					{
						commit = ((PlSqlJobInfo) job).isCommit();
						function = ((PlSqlJobInfo) job).isFunction();
					}
					else
						return RESULT_Error;
				}
				else
				{
					query = queryStr != null;
					function = functionStr != null;
					commit = commitStr != null;
					if (sql == null)
						sql = "";
					if (input == null)
						input = "";
					if (output == null)
						output = "";
				}
			}
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
		return RESULT_Ok;
	}

	public String getProviderName()
	{
		return providerName;
	}

	public String getName()
	{
		return name;
	}

	public String getType()
	{
		return type;
	}

	public boolean isQuery()
	{
		return query;
	}

	public String getAddress()
	{
		return address;
	}

	public String getAlias()
	{
		return alias;
	}

	public String getSql()
	{
		return sql;
	}

	public String getInput()
	{
		return input;
	}

	public String getOutput()
	{
		return output;
	}

	public boolean isCreating()
	{
		return isCreating;
	}

	public boolean isCommit()
	{
		return commit;
	}

	public boolean isFunction()
	{
		return function;
	}

	public MessageSet getMessages() throws Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		return job.getMessages();
	}
}
