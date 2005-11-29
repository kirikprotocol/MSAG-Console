package ru.novosoft.smsc.admin.service;

import ru.novosoft.smsc.admin.AdminException;

import java.io.File;
import java.util.Collection;
import java.util.List;
import java.util.Map;

/**
 * Created by igork
 * Date: Jan 21, 2003
 * Time: 6:49:26 PM
 */
public interface ServiceManager
{
  void addAllInfos(Map services) throws AdminException;

  boolean contains(String id);

  void deployAdministrableService(File incomingZip, ServiceInfo serviceInfo, File[] serviceFolder) throws AdminException;

  Service get(String serviceId) throws AdminException;

  ServiceInfo getInfo(String serviceId) throws AdminException;

  /**
   * @return Service IDs
   */
  List getServiceIds();

  boolean isServiceAdministrable(String serviceId);

  Service remove(String serviceId) throws AdminException;

  void removeAll(Collection serviceIds) throws AdminException;

  void rollbackDeploy(String serviceId, File[] serviceFolder);

  void updateServices(Map services);

  Service add(Service newService) throws AdminException;
}
