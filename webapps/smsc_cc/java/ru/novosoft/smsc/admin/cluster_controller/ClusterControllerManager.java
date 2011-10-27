package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;

import java.util.List;

/**
 * User: artem
 * Date: 26.10.11
 */
public interface ClusterControllerManager {
  ClusterControllerSettings getSettings() throws AdminException;

  void updateSettings(ClusterControllerSettings newSettings) throws AdminException;

  void startClusterController() throws AdminException;

  void stopClusterController() throws AdminException;

  void switchClusterController(String toHost) throws AdminException;

  String getControllerOnlineHost() throws AdminException;

  List<String> getControllerHosts() throws AdminException;
}
