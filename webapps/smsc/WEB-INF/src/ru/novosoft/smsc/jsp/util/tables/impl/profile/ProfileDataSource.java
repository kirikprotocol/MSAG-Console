/*
 * Created by igork
 * Date: 28.08.2002
 * Time: 18:23:21
 */
package ru.novosoft.smsc.jsp.util.tables.impl.profile;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.util.tables.*;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;
import ru.novosoft.util.conpool.NSConnectionPool;

import java.sql.*;
import java.util.Iterator;

public class ProfileDataSource implements DataSource
{
	private static final String[] columnNames = {"Mask", "Codepage", "Report info"};

	NSConnectionPool connectionPool = null;

	public ProfileDataSource(NSConnectionPool connectionPool) throws AdminException
	{
		this.connectionPool = connectionPool;
	}

	public QueryResultSet query(ProfileQuery query_to_run) throws AdminException
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

			sqlResultSet = statement.executeQuery("select mask, reportinfo, codeset, locale from sms_profile " + createWhereStatement((ProfileFilter) query_to_run.getFilter()) + " order by " + sortOrder);

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
				String locale = sqlResultSet.getString("locale");
				results.add(new ProfileDataItem(new Profile(new Mask(maskString), codeset, reportinfo, locale)));
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

	public QueryResultSet query(Query query_to_run) throws AdminException
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

			if (!filter.getMasks().isEmpty())
			{
				result += '(';
				for (Iterator i = filter.getMasks().iterator(); i.hasNext();)
				{
					Mask mask = (Mask) i.next();
					result += "mask like '" + mask.getNormalizedMask() + "%'" + (i.hasNext() ? " or " : "");
				}
				result += ')';
			}

			if (filter.getCodepage() >= 0)
				result += (result.length() > 0 ? " and " : "") + "codeset = " + filter.getCodepage();

			if (filter.getReportinfo() >= 0)
				result += (result.length() > 0 ? " and " : "") + "reportinfo = " + filter.getReportinfo();

			if (!filter.getLocales().isEmpty())
			{
				result += (result.length() > 0 ? " and " : "") + "locale in (";
				for (Iterator i = filter.getLocales().iterator(); i.hasNext();)
				{
					String locale = (String) i.next();
					result += '\'' + locale + '\'' + (i.hasNext() ? ", " : "");
				}
				result += ')';
			}

			return (result.length() > 0 ? ("where " + result) : "");
		}
	}
}
