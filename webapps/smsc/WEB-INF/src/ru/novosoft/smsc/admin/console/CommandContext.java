/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 19, 2002
 * Time: 1:44:55 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console;

import ru.novosoft.smsc.admin.smsc_service.*;

import java.util.ArrayList;
import java.util.Collection;

public class CommandContext
{
	public final static int CMD_OK = 100;
	public final static int CMD_LIST = 200;
	public final static int CMD_AUTH_ERROR = 500;
	public final static int CMD_PARSE_ERROR = 600;
	public final static int CMD_PROCESS_ERROR = 700;

	protected Smsc smsc;
	protected SmeManager smeManager;
	protected RouteSubjectManager routeSubjectManager;
	protected int status = CMD_OK;
	protected String message = "No message";
	protected ArrayList results = new ArrayList();

	public CommandContext(Smsc smsc, SmeManager smeManager, RouteSubjectManager routeSubjectManager)
	{
		this.smsc = smsc;
		this.smeManager = smeManager;
		this.routeSubjectManager = routeSubjectManager;
	}

	public Smsc getSmsc()
	{
		return smsc;
	}

	public SmeManager getSmeManager()
	{
		return smeManager;
	}

	public RouteSubjectManager getRouteSubjectManager()
	{
		return routeSubjectManager;
	}

	public String getMessage()
	{
		return message;
	}

	public void setMessage(String message)
	{
		this.message = message;
	}

	public int getStatus()
	{
		return status;
	}

	public void setStatus(int status)
	{
		this.status = status;
	}

	public void addResult(Object res)
	{
		results.add(res);
	}

	public void addResults(Collection col)
	{
		results.addAll(col);
	}

	public Collection getResults()
	{
		return results;
	}
}
