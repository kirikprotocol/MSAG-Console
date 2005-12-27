package ru.novosoft.smsc.admin.service;

/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 5:20:22 PM
 */

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.util.WebAppFolders;

import java.io.File;
import java.util.HashMap;
import java.util.Map;


public class ServiceInfo {
	public static final byte STATUS_UNKNOWN = -1;
	/* All other statuses except online should be negative values*/
	public static final byte STATUS_OFFLINE = 0;
	public static final byte STATUS_ONLINE1 = 1;
	public static final byte STATUS_ONLINE2 = 2;
	public static final byte STATUS_ONLINE = 127;

	protected String id = "";
	protected String host = "";
	protected String args = "";
	protected Map components = new HashMap();
	protected SME sme = null;
	protected byte status = STATUS_OFFLINE;
	private File serviceFolder;
	private boolean autostart;


	public ServiceInfo(final Element serviceElement, final String serviceHost, final SmeManager smeManager, final String daemonServicesFolder)
			throws AdminException {
		host = serviceHost;
		id = serviceElement.getAttribute("id");
		this.autostart = "true".equals(serviceElement.getAttribute("autostart"));
		args = serviceElement.getAttribute("args");

		if ("".equals(id)) {
			throw new AdminException("services name or services system id not specified in response");
		}
		if (id.equals(Constants.SMSC_SME_ID)) {
			if (smeManager.contains(id)) {
				throw new AdminException("Couldn't add new SMSC - already presented");
			}
			sme = new SME(id, 0, SME.SMPP, 0, 0, 0, "", "", "", 0, false, false, 0, "", false, SME.MODE_TRX, 0, 0);
		}
		else {
			if (id.equals(Constants.ARCHIVE_DAEMON_SVC_ID)) {
				if (smeManager.contains(id)) {
					throw new AdminException("Couldn't add new ArchiveDaemon - already presented");
				}
				sme = new SME(id, 0, SME.SMPP, 0, 0, 0, "", "", "", 0, false, false, 0, "", false, SME.MODE_TRX, 0, 0);
			}
			else {
				sme = smeManager.get(id);
			}
		}

		setStatusStr(serviceElement.getAttribute("status"));
//? id==folder
		this.serviceFolder = new File(daemonServicesFolder, id);
	}

	public ServiceInfo(final String servId, final SmeManager smeManager, final byte status) throws AdminException {
		id = servId;
		if ("".equals(id)) {
			throw new AdminException("services name or services system id not specified in response");
		}
		if (id.equals(Constants.SMSC_SME_ID)) {
			if (smeManager.contains(id)) {
				throw new AdminException("Couldn't add new SMSC - already presented");
			}
			sme = new SME(id, 0, SME.SMPP, 0, 0, 0, "", "", "", 0, false, false, 0, "", false, SME.MODE_TRX, 0, 0);
		}
		else {
			if (id.equals(Constants.ARCHIVE_DAEMON_SVC_ID)) {
				if (smeManager.contains(id)) {
					throw new AdminException("Couldn't add new ArchiveDaemon - already presented");
				}
				sme = new SME(id, 0, SME.SMPP, 0, 0, 0, "", "", "", 0, false, false, 0, "", false, SME.MODE_TRX, 0, 0);
			}
			else {
				try {
					sme = smeManager.get(id);
				}
				catch (AdminException e) {
					sme = new SME(id, 0, SME.SMPP, 0, 0, 0, "", "", "", 0, false, false, 0, "", false, SME.MODE_TRX, 0, 0);
				}
			}
		}
		this.status = status;
		this.serviceFolder = new File(WebAppFolders.getHAServicesFolder(), servId);
	}

	public ServiceInfo(final String id, final String host, final String serviceFolder, final String args, final boolean autostart, final SME sme,
					   final byte status) {
		this.host = host;
		this.args = args;
		this.autostart = autostart;
		this.id = id;
		this.sme = sme;
		this.status = status;
		this.serviceFolder = new File(serviceFolder);
	}

	public String getHost() {
		return host;
	}

	public String getId() {
		return id;
	}

	public void setId(final String id) {
		this.id = id;
	}


	public String getArgs() {
		return args;
	}

	public Map getComponents() {
		return components;
	}

	public void setComponents(final Element response) {
		components.clear();
		final NodeList list = response.getElementsByTagName("component");
		for (int i = 0; i < list.getLength(); i++) {
			final Element compElem = (Element) list.item(i);
			final Component c = new Component(compElem);
			components.put(c.getName(), c);
		}
	}

	public SME getSme() {
		return sme;
	}

	public byte getStatus() {
		return status;
	}

	protected void setStatusStr(final String statusStr) {
		if ("running".equalsIgnoreCase(statusStr) || "starting".equalsIgnoreCase(statusStr)) {
			this.status = STATUS_ONLINE;
		}
		else {
			if ("stopping".equalsIgnoreCase(statusStr) || "stopped".equalsIgnoreCase(statusStr)) {
				this.status = STATUS_OFFLINE;
			}
			else {
				this.status = STATUS_UNKNOWN;
			}
		}
	}

	public String getStatusStr() {
		switch (status) {
		case STATUS_ONLINE1:
		case STATUS_ONLINE2:
		case STATUS_ONLINE:
			return "running";
		case STATUS_OFFLINE:
			return "stopped";
		default:
			return "unknown";
		}
	}

	public SME setSme(final SME sme) {
		final SME old_sme = this.sme;
		this.sme = sme;
		return old_sme;
	}

	public void setStatus(final byte status) {
		this.status = status;
	}

	public void setArgs(final String args) {
		this.args = args;
	}

	public File getServiceFolder() {
		return serviceFolder;
	}

	public boolean isAutostart() {
		return autostart;
	}

	public boolean isOnline() {
		return this.status > 0;
	}
}
