/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 15, 2002
 * Time: 2:19:31 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.admin.smsc_service.*;
import ru.novosoft.smsc.admin.console.human.HumanGate;
import ru.novosoft.smsc.admin.console.script.ScriptGate;
import ru.novosoft.smsc.util.config.Config;

import java.util.ArrayList;
import java.util.ResourceBundle;
import java.util.StringTokenizer;
import java.io.IOException;

public class Console
{
	private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());

	private Smsc smsc = null;
	private SmeManager smeManager = null;
	private RouteSubjectManager routeSubjectManager = null;
	private Gate humanGate = null;
	private Gate scriptGate = null;

	private ArrayList sessions = new ArrayList();

	private ResourceBundle commandRolesBundle = null;

	public Console(SMSCAppContext context) throws IOException, Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		if (context != null)
		{
			this.smsc = context.getSmsc();
			this.smeManager = context.getSmeManager();
			this.routeSubjectManager = context.getRouteSubjectManager();
			int humanPort = context.getConfig().getInt("console.humanPort");
			int scriptPort = context.getConfig().getInt("console.scriptPort");
			commandRolesBundle = ResourceBundle.getBundle("ru.novosoft.smsc.admin.console.commands.roles");
			humanGate = new HumanGate(this, humanPort);
			scriptGate = new ScriptGate(this, scriptPort);
			//this.start();
		}
	}

	public void start()
	{
		logger.debug("Starting administration console ...");
		humanGate.start();
		scriptGate.start();
		logger.debug("Administration console started.");
	}

	public void close()
	{
		logger.debug("Closing administration console ...");
		humanGate.close();
		scriptGate.close();
		while (sessions.size() > 0)
		{
			((Session) sessions.get(0)).close();
		}
		logger.debug("Administration console closed.");
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

	public void addSession(Session session)
	{
		synchronized (sessions)
		{
			sessions.add(session);
		}
	}

	public void removeSession(Session session)
	{
		synchronized (sessions)
		{
			sessions.remove(session);
		}
	}

	public String[] getCommandRoles(String command)
	{
		if (commandRolesBundle == null) return null;
		String roles = commandRolesBundle.getString(command);
		if (roles == null) return null;

		StringTokenizer st = new StringTokenizer(roles, ",");
		String out[] = new String[st.countTokens()];
		for (int i = 0; st.hasMoreTokens(); i++)
		{
			out[i] = st.nextToken();
		}

		return out;
	}
}
