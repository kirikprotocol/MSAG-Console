/*
 * Created by igork
 * Date: 28.08.2002
 * Time: 18:23:21
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.util.tables.DataSource;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.util.conpool.NSConnectionPool;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

public class ProfileDataSource implements DataSource
{
	private static final String[] columnNames = {"Mask", "Codepage", "Report info"};

	NSConnectionPool connectionPool = null;

	public ProfileDataSource(NSConnectionPool connectionPool)
			  throws AdminException
	{
		this.connectionPool = connectionPool;
	}

	public QueryResultSet query(ProfileQuery query_to_run)
			  throws AdminException
	{
		String sort = (String) query_to_run.getSortOrder().get(0);
		boolean isNegativeSort = false;
		if (sort.charAt(0) == '-')
		{
			sort = sort.substring(1);
			isNegativeSort = true;
		}

		Connection connection = null;
		Statement statement = null;
		ResultSet sqlResultSet = null;
		QueryResultSetImpl results = null;

		try
		{
			String sortOrder;
			if (!sort.equalsIgnoreCase("mask"))
				sortOrder = sort + (isNegativeSort ? " desc" : " asc") + ", mask asc";
			else
				sortOrder = sort + (isNegativeSort ? " desc" : " asc");

			// open connection to SQL server and run query
			connection = connectionPool.getConnection();
			statement = connection.createStatement();

			sqlResultSet = statement.executeQuery("select mask, reportinfo, codeset from sms_profile " + createWhereStatement((ProfileFilter) query_to_run.getFilter()) + " order by " + sortOrder);

			int totalCount = 0;
			// skip lines to start position
			for (int i = 0; i < query_to_run.getStartPosition() && sqlResultSet.next(); i++, totalCount++) ;

			// retrieve data
			results = new QueryResultSetImpl(columnNames, query_to_run.getSortOrder());
			for (int i = 0; i < query_to_run.getExpectedResultsQuantity() && sqlResultSet.next(); i++, totalCount++)
			{
				//System.out.println("Query: ");
				String maskString = sqlResultSet.getString("mask");
				//System.out.println("maskString = " + maskString);
				byte reportinfo = sqlResultSet.getByte("reportinfo");
				//System.out.println("reportinfo = " + reportinfo);
				byte codeset = sqlResultSet.getByte("codeset");
				//System.out.println("codeset = " + codeset);
				results.add(new ProfileDataItem(new Profile(new Mask(maskString), codeset, reportinfo)));
			}

			boolean isLast = true;
			while (sqlResultSet.next())
			{
				totalCount++;
				isLast = false;
			}
			results.setTotalSize(totalCount);
			results.setLast(isLast);
		}
		catch (SQLException e)
		{
			throw new AdminException("Couldn't retrieve data from profiles database: " + e.getMessage());
		}
		finally
		{
			// close SQL connection
			if (sqlResultSet != null)
				try
				{
					sqlResultSet.close();
				}
				catch (SQLException e)
				{
				}
			if (statement != null)
				try
				{
					statement.close();
				}
				catch (SQLException e)
				{
				}
			if (connection != null)
				try
				{
					connection.close();
				}
				catch (SQLException e)
				{
				}
		}
		if (results != null)
			return results;
		else
			throw new AdminException("Couldn't retrieve data from profiles database");
	}

	public QueryResultSet query(Query query_to_run)
			  throws AdminException
	{
		return query((ProfileQuery) query_to_run);
	}

	private String createWhereStatement(ProfileFilter filter)
	{
		if (filter.isEmpty())
			return "";
		else
		{
			String result = "";
			if (filter.getMasks().length > 0)
			{
				result += '(';
				for (int i = 0; i < filter.getMasks().length; i++)
				{
					String mask = filter.getMasks()[i];
					result += (i == 0 ? "" : " or ") + "mask like '" + mask + "%'";
				}
				result += ')';
			}
			if (filter.getCodepage() >= 0)
				result += (result.length() > 0 ? " and " : "") + "codeset = " + filter.getCodepage();
			if (filter.getReportinfo() >= 0)
				result += (result.length() > 0 ? " and " : "") + "reportinfo = " + filter.getReportinfo();

			return (result.length() > 0 ? ("where " + result) : "");
		}
	}
}
