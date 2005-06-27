/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 15, 2002
 * Time: 2:19:31 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.Statuses;
import ru.novosoft.smsc.admin.smsc_service.*;
import ru.novosoft.smsc.admin.console.human.HumanGate;
import ru.novosoft.smsc.admin.console.script.ScriptGate;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.journal.Journal;
import ru.novosoft.smsc.util.config.Config;

import java.util.ArrayList;
import java.util.ResourceBundle;
import java.util.StringTokenizer;
import java.io.IOException;

public class Console extends Thread
{
	private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());

	private SMSCAppContext context = null;

	private Gate humanGate = null;
	private Gate scriptGate = null;

	private final static int DEFAULT_INACTIVITY_TIMEOUT = 60;
	private long inactivityTimeout = DEFAULT_INACTIVITY_TIMEOUT*1000;

	private ArrayList sessions = new ArrayList();

	private ResourceBundle commandRolesBundle = null;

	public Console(SMSCAppContext context)
            throws IOException, Config.ParamNotFoundException, Config.WrongParamTypeException
	{
		if (context != null)
		{
			this.context = context; 
			Config cfg = context.getConfig();
			int humanPort = cfg.getInt("console.humanPort");
			int scriptPort = cfg.getInt("console.scriptPort");

			try { inactivityTimeout = cfg.getInt("console.timeout"); }
			catch (Config.ParamNotFoundException ce) {
				inactivityTimeout = DEFAULT_INACTIVITY_TIMEOUT;
				logger.warn("Missed <console.timeout> parameter. Using default: "+DEFAULT_INACTIVITY_TIMEOUT+" sec");
			} inactivityTimeout *= 1000; // convert to msecs

			commandRolesBundle = ResourceBundle.getBundle("ru.novosoft.smsc.admin.console.commands.roles");
			humanGate = new HumanGate(this, humanPort);
			scriptGate = new ScriptGate(this, scriptPort);
		}
	}

  boolean isStopping = false;
  Object closeSemaphore = new Object();
  Object sessionMonitor = new Object();

  public void start()
  {
      logger.debug("Starting administration console ...");
      humanGate.start(); scriptGate.start(); super.start();
      logger.debug("Administration console started.");
  }

  public void close()
  {
      logger.debug("Closing administration console ...");
      synchronized(closeSemaphore) { // stop this thread
          isStopping = true;
          synchronized(sessionMonitor) { sessionMonitor.notifyAll(); }
          try { closeSemaphore.wait(); }
          catch (InterruptedException e) { }
      }
      humanGate.close(); scriptGate.close();
      while (sessions.size() > 0)	{
          Session session = (Session) sessions.get(0);
          try { session.farewell(" Console server stopped"); sleep(500); } catch (Exception e) { }
          session.close(true);
      }
      logger.debug("Administration console closed.");
  }

  public void run()
  {
    while (!isStopping)
    {
      synchronized(sessionMonitor)
      {
        try { sessionMonitor.wait(inactivityTimeout/2); }
        catch (InterruptedException e) { break; }

        for (int i=0; i<sessions.size(); i++)
        {
          Session session = (Session) sessions.get(i);
          if (session.isTimedOut(inactivityTimeout)) {
            logger.warn("Console session is timed out");
            try { session.farewell(" Session is timed out."); sleep(500); } catch (Exception e) { }
            sessions.remove(session);
            session.close(false);
          }
        }
      }
    }
    synchronized(closeSemaphore) {
      closeSemaphore.notifyAll();
    }
  }

    public void addSession(Session session) {
      synchronized (sessionMonitor)	{ sessions.add(session); }
    }
    public void removeSession(Session session) {
      synchronized (sessionMonitor) { sessions.remove(session);	}
    }

	public Smsc getSmsc() {
		return context.getSmsc();
	}
	public SmeManager getSmeManager() {
		return context.getSmeManager();
	}
	public RouteSubjectManager getRouteSubjectManager() {
		return context.getRouteSubjectManager();
	}
    public CategoryManager getCategoryManager() {
        return context.getCategoryManager();
    }
    public ProviderManager getProviderManager() {
        return context.getProviderManager();
    }
    public Config getWebappConfig() {
        return context.getConfig();
    }

	public Journal getJournal() {
		return context.getJournal();
	}

	public Statuses getStatuses() {
		return context.getStatuses();
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
