/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 4:05:54 PM
 */
package ru.novosoft.smsc.admin.daemon;

import org.apache.log4j.Category;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.protocol.*;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.utli.Proxy;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.HashMap;
import java.util.Map;


public class Daemon extends Proxy
{
	private Socket socket = null;
	private OutputStream out;
	private InputStream in;
	private CommandWriter writer;
	private ResponseReader reader;
	private Category logger = Category.getInstance(this.getClass().getName());
	private Smsc smsc = null;
	private boolean containsSmsc = false;

	public Daemon(String host, int port, Smsc smsc)
			throws AdminException
	{
		super(host, port);
		this.smsc = smsc;
		connect(host, port);
	}

	/**
	 * @return Process ID (PID) of new services
	 */
	public synchronized long startService(String serviceId)
			throws AdminException
	{
		Response r = runCommand(new CommandStartService(serviceId));
		if (r.getStatus() != Response.StatusOk)
			throw new AdminException("Couldn't start services \"" + serviceId + "\", nested:" + r.getDataAsString());

		String pidStr = r.getDataAsString().trim();
		try
		{
			return Long.decode(pidStr).longValue();
		}
		catch (NumberFormatException e)
		{
			throw new AdminException("PID of new services misformatted (" + pidStr + "), nested:" + e.getMessage());
		}
	}

	public synchronized void addService(ServiceInfo serviceInfo)
			throws AdminException
	{
		logger.debug("Add services \"" + serviceInfo.getId() + "\" (" + serviceInfo.getHost() + ':'
						 + serviceInfo.getPort() + ")");

		Response r = runCommand(new CommandAddService(serviceInfo));
		if (r.getStatus() != Response.StatusOk)
			throw new AdminException("Couldn't add services \"" + serviceInfo.getId() + '/' + serviceInfo.getId()
											 + "\" [" + serviceInfo.getArgs() + "], nested:" + r.getDataAsString());
	}

	public synchronized void removeService(String serviceId)
			throws AdminException
	{
		Response r = runCommand(new CommandRemoveService(serviceId));
		if (r.getStatus() != Response.StatusOk)
			throw new AdminException("Couldn't remove services \"" + serviceId + "\", nested:" + r.getDataAsString());
	}

	public synchronized void shutdownService(String serviceId)
			throws AdminException
	{
		Response r = runCommand(new CommandShutdownService(serviceId));
		if (r.getStatus() != Response.StatusOk)
			throw new AdminException("Couldn't shutdown services \"" + serviceId + "\", nested:" + r.getDataAsString());
	}

	public synchronized void killService(String serviceId)
			throws AdminException
	{
		Response r = runCommand(new CommandKillService(serviceId));
		if (r.getStatus() != Response.StatusOk)
			throw new AdminException("Couldn't kill services \"" + serviceId + "\", nested:" + r.getDataAsString());
	}

	/**
	 * Queries demon for services list
	 * @return Map: services name -> ServiceInfo
	 */
	public synchronized Map listServices()
			throws AdminException
	{
		Response r = runCommand(new CommandListServices());
		if (r.getStatus() != Response.StatusOk)
			throw new AdminException("Couldn't list services, nested:" + r.getDataAsString());

		Map result = new HashMap();
		containsSmsc = false;

		NodeList list = r.getData().getElementsByTagName("service");
		for (int i = 0; i < list.getLength(); i++)
		{
			final Element serviceElement = (Element) list.item(i);
			String serviceId = serviceElement.getAttribute("id");
			containsSmsc |= Constants.SMSC_SME_ID.equals(serviceId);
			ServiceInfo newInfo = new ServiceInfo(serviceElement, host, smsc.getSmes());
			result.put(newInfo.getId(), newInfo);
		}

		return result;
	}

	public synchronized void setServiceStartupParameters(String serviceId, /*String serviceName, */int port, String args)
			throws AdminException
	{
		Response r = runCommand(new CommandSetServiceStartupParameters(serviceId, /*serviceName, */port, args));
		if (r.getStatus() != Response.StatusOk)
			throw new AdminException("Couldn't set services startup parameters \"" + serviceId + "\", nested:" + r.getDataAsString());
	}

	public boolean isContainsSmsc()
	{
		return containsSmsc;
	}
}
