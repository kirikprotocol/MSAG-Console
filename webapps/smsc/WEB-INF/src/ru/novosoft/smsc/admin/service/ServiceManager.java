package ru.novosoft.smsc.admin.service;

import ru.novosoft.smsc.admin.AdminException;

import java.io.File;
import java.util.*;

/**
 * Created by igork
 * Date: Jan 21, 2003
 * Time: 6:49:26 PM
 */
public interface ServiceManager
{
	void addAll(Map services) throws AdminException;

	boolean contains(String id);

	void deployAdministrableService(File incomingZip, ServiceInfo serviceInfo) throws AdminException;

	Service get(String serviceId) throws AdminException;

	ServiceInfo getInfo(String serviceId) throws AdminException;

	/**
	 *
	 * @return Service IDs
	 */
	List getServiceIds();

	boolean isServiceAdministarble(String serviceId);

	Service remove(String serviceId) throws AdminException;

	void removeAll(Collection serviceIds) throws AdminException;

	void rollbackDeploy(String hostName, String serviceId);

	void updateServices(Map services);
}
