/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 19, 2002
 * Time: 1:44:55 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console;

import ru.novosoft.smsc.admin.smsc_service.*;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.journal.Journal;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.jsp.Statuses;

import java.util.ArrayList;
import java.util.Collection;

public class CommandContext
{
	public final static int CMD_OK = 100;
    public final static int CMD_LIST = 200;
    public final static int CMD_WARNING = 300;
	public final static int CMD_AUTH_ERROR = 500;
	public final static int CMD_PARSE_ERROR = 600;
	public final static int CMD_PROCESS_ERROR = 700;

	protected Smsc smsc;
	protected SmeManager smeManager;
	protected RouteSubjectManager routeSubjectManager;
	protected CategoryManager categoryManager;
	protected ProviderManager providerManager;
	protected Journal journal;
	protected Statuses statuses;
	private Config webappConfig = null;

	protected int status = CMD_OK;
	protected String message = "No message";
	protected ArrayList results = new ArrayList();

	public CommandContext(Console owner)
	{
		this.smsc = owner.getSmsc();
		this.smeManager = owner.getSmeManager();
		this.routeSubjectManager = owner.getRouteSubjectManager();
		this.categoryManager = owner.getCategoryManager();
		this.providerManager = owner.getProviderManager();
		this.webappConfig = owner.getWebappConfig();
		this.journal = owner.getJournal();
		this.statuses = owner.getStatuses();
	}

	public Smsc getSmsc() {
		return smsc;
	}
	public Journal getJournal() {
		return journal;
	}
	public Statuses getStatuses() {
		return statuses;
	}
	public SmeManager getSmeManager() {
		return smeManager;
	}
	public RouteSubjectManager getRouteSubjectManager() {
		return routeSubjectManager;
	}
    public CategoryManager getCategoryManager() {
        return categoryManager;
    }
    public ProviderManager getProviderManager() {
        return providerManager;
    }
    public Config getSmscConfig() {
        return (smsc == null) ? null : smsc.getSmscConfig();
    }
    public Config getWebappConfig() {
        return webappConfig;
    }

	public String getMessage(){
		return message;
	}
	public void setMessage(String message){
		this.message = message;
	}

	public int getStatus(){
		return status;
	}
	public void setStatus(int status) {
		this.status = status;
	}

	public void addResult(Object res) {
		results.add(res);
	}
	public void addResults(Collection col) {
		results.addAll(col);
	}
	public Collection getResults() {
		return results;
	}
}
