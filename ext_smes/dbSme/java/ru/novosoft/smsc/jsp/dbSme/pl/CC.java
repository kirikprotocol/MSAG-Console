/*
 * Created by igork
 * Date: 07.10.2002
 * Time: 21:16:30
 */
package ru.novosoft.smsc.jsp.dbSme.pl;

import ru.novosoft.smsc.admin.utli.Functions;
import ru.novosoft.smsc.jsp.dbSme.bl.DBSMEConfig;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.File;

public abstract class CC
{
	public static final String DBSME_ID = "dbSme";

	public static final int RESULT_Ok = 0;
	public static final int RESULT_Error = 1;
	public static final int RESULT_Done = 2;
	public static final int RESULT_NotAllParams = 3;

	protected DBSMEConfig config = null;

	public int process(HttpServletRequest request, HttpServletResponse response)
			throws Exception
	{
		config = new DBSMEConfig(new File(new File(Functions.getAppContext(request).getSuperManager().getServiceFolder(DBSME_ID), "conf"), "config.xml"));
		return loadData(request);
	}

	protected abstract int loadData(HttpServletRequest request);
}
