package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.WebAppFolders;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;

/**
 * Created by igork
 * Date: Jul 16, 2003
 * Time: 4:50:30 PM
 */
public class DbSmeContext {
	private static DbSmeContext dbSmeContext = null;

	public static synchronized DbSmeContext getInstance(SMSCAppContext appContext)
	{
		if (dbSmeContext == null)
			dbSmeContext = new DbSmeContext(appContext);
		return dbSmeContext;
	}

	private boolean configChanged = false;

	public DbSmeContext(SMSCAppContext appContext)
	{
		this.configChanged = false;
		File tempConfigFile = new File(WebAppFolders.getWorkFolder(), "dbSme.config.xml.tmp");
		if (tempConfigFile.exists()) {
			File origConfigFile = null;
			try {
				origConfigFile = new File(WebAppFolders.getServiceFolder(appContext.getHostsManager().getServiceInfo(Constants.DBSME_SME_ID).getHost(), Constants.DBSME_SME_ID), "conf/config.xml");
			} catch (Exception e) {
				System.out.println("Couldn't get original config, nested: " + e.getMessage());
			}
         if (origConfigFile.exists())
			{
				if (origConfigFile.length() != tempConfigFile.length())
				{
					this.configChanged = true;
					return ;
				}
				try {
					InputStream temp = new BufferedInputStream(new FileInputStream(tempConfigFile));
					InputStream orig = new BufferedInputStream(new FileInputStream(origConfigFile));
					for (int readedTemp = temp.read(); readedTemp != -1; readedTemp = temp.read())
					{
						if (readedTemp != orig.read())
						{
							configChanged = true;
							temp.close();
							orig.close();
							return;
						}
					}
				} catch (Exception e) {
					System.out.println("Couldn't compare origianl and saved configs, nested: " + e.getMessage());
				}
			}
		}
	}

	public boolean isConfigChanged()
	{
		return configChanged;
	}

	public void setConfigChanged(boolean configChanged)
	{
		this.configChanged = configChanged;
	}
}
