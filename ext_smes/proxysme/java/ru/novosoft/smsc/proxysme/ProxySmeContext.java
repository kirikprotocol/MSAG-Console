package ru.novosoft.smsc.proxysme;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.AdminException;

import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.IOException;

import org.xml.sax.SAXException;

/**
 * Created by igork
 * Date: Aug 5, 2003
 * Time: 3:56:21 PM
 */
public class ProxySmeContext {
	private static ProxySmeContext instance = null;

	public static ProxySmeContext getInstance(SMSCAppContext appContext) throws AdminException, IOException, ParserConfigurationException, SAXException
	{
		return instance == null ? (instance = new ProxySmeContext(appContext)) : instance;
	}


	private SMSCAppContext appContext = null;
	private Config config = null;

	public ProxySmeContext(SMSCAppContext appContext) throws AdminException, IOException, ParserConfigurationException, SAXException
	{
		this.appContext = appContext;
		this.config = loadConfig();
	}

	public Config getConfig()
	{
		return config;
	}

	public Config loadConfig() throws AdminException, SAXException, ParserConfigurationException, IOException
	{
    return config = new Config(new File(appContext.getHostsManager().getServiceInfo(Constants.PROXY_SME_ID).getServiceFolder(),
														"conf" + File.separatorChar + "config.xml"));
	}
}
