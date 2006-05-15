package ru.novosoft.smsc.jsp.smsc.services;

/*
 * Created by igork
 * Date: 29.10.2002
 * Time: 23:12:47
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.SmeStatus;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.PageBean;

import java.util.List;

public class Statuses extends PageBean {
	protected boolean colored = true;

	public List getServiceIds() {
		return appContext.getSmeManager().getSmeNames();
	}

	public byte getServiceStatus(String id) {
		try {
			if (hostsManager.isService(id)) {
				return hostsManager.getServiceInfo(id).getStatus();
			}
			else {
				return ServiceInfo.STATUS_UNKNOWN;
			}
		}
		catch (AdminException e) {
			logger.error("Couldn't get service \"" + id + "\" status", e);
			return ServiceInfo.STATUS_UNKNOWN;
		}
	}

	public SmeStatus getSmeStatus(String id) {
		try {
			return appContext.getSmeManager().smeStatus(id);
		}
		catch (AdminException e) {
			logger.error("Couldn't get service \"" + id + "\" connection status, nested: " + e.getMessage());
			return null;
		}
	}

	public boolean isColored() {
		return colored;
	}

	public void setColored(boolean colored) {
		this.colored = colored;
	}

	public String smscServStatusString(String serviceId, byte nodeId) {
		byte status = ServiceInfo.STATUS_UNKNOWN;
		try {
			status = appContext.getHostsManager().getServiceInfo(serviceId).getStatus();
		}
		catch (Throwable e) {
		}
		switch (status) {
		case ServiceInfo.STATUS_OFFLINE:
			return appContext.getLocaleString(principal, "common.statuses.deactivated");
		case ServiceInfo.STATUS_ONLINE1:
			if (nodeId == ServiceInfo.STATUS_ONLINE1) {
				return appContext.getLocaleString(principal, "common.statuses.activated");
			}
			else {
				return appContext.getLocaleString(principal, "common.statuses.deactivated");
			}
		case ServiceInfo.STATUS_ONLINE2:
			if (nodeId == ServiceInfo.STATUS_ONLINE2) {
				return appContext.getLocaleString(principal, "common.statuses.activated");
			}
			else {
				return appContext.getLocaleString(principal, "common.statuses.deactivated");
			}
		default:
			return appContext.getLocaleString(principal, "common.statuses.unknown");
		}
	}
}
