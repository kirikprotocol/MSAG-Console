package ru.novosoft.smsc.admin.route;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.CCRouteTrace;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;

import java.io.*;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * @author Artem Snopkov
 */
public class RouteSubjectManagerImpl implements RouteSubjectManager {

  private final ConfigFileManager<RouteSubjectSettings> cfgFileManager;
  private final ClusterController cc;
  private final AtomicInteger routeTracerId = new AtomicInteger(0);

  public RouteSubjectManagerImpl(File configFile, File backupDir, FileSystem fs, ClusterController cc) {
    this.cfgFileManager = new ConfigFileManager<RouteSubjectSettings>(configFile, backupDir, fs, new RoutesConfig(this));
    this.cc = cc;
  }

  public RouteSubjectSettings getSettings() throws AdminException {
    try {
      if (cc.isOnline())
        cc.lockRoutes(false);

      return cfgFileManager.load();
    } finally {
      if (cc.isOnline())
        cc.unlockRoutes();
    }
  }

  public void updateSettings(RouteSubjectSettings settings) throws AdminException {
    try {
      if (cc.isOnline())
        cc.lockRoutes(false);

      cfgFileManager.save(settings);

      if (cc.isOnline())
        cc.applyRoutes();
    } finally {
      if (cc.isOnline())
        cc.unlockRoutes();
    }
  }

  private File getRouteTracingConfig() {
    File routesTracingConfig = null;
    while (routesTracingConfig == null) {
      int traceId = routeTracerId.incrementAndGet();
      routesTracingConfig = new File(cfgFileManager.getConfigFile().getParent(), "routes_" + traceId + ".xml");
      if (routesTracingConfig.exists())
        routesTracingConfig = null;
    }
    return routesTracingConfig;
  }

  /**
   * Производит трассировку маршрутов. Порядок трассировки следующий:
   * 1. Конфигурация маршрутов сохраняется во временный файл. Он должен находиться в той же директории, что и основной файл с маршрутами.
   * 2. В Cluster Controller отправляется команда на трассировку маршрута. В параметрах команды указывается имя файла с маршрутами
   * 3. После окончания трассировки временный файл с маршрутами удаляется.
   *
   * @param settings    настройки маршрутов
   * @param source      адрес отправителя
   * @param destination адрес получателя
   * @param sourceSmeId smeId отправителя
   * @return объект RouteTrace с результатами
   * @throws AdminException если произошла ошибка
   */
  RouteTrace traceRoute(RouteSubjectSettings settings, Address source, Address destination, String sourceSmeId) throws AdminException {
    if (!cc.isOnline())
      throw new RouteException("route.tracing.unavailable");

    File routesTracingConfig = getRouteTracingConfig();

    try {
      InputStream is = null;
      OutputStream os = null;
      try {
        is = cfgFileManager.getFileSystem().getInputStream(cfgFileManager.getConfigFile());
        os = cfgFileManager.getFileSystem().getOutputStream(routesTracingConfig);
        new RoutesConfig(this).save(is, os, settings);
      } catch (Exception e) {
        throw new RouteException("route.tracing.error", e);
      } finally {
        if (is != null)
          try {
            is.close();
          } catch (IOException ignored) {
          }
        if (os != null)
          try {
            os.close();
          } catch (IOException ignored) {
          }
      }

      CCRouteTrace trace = cc.traceRoute(routesTracingConfig.getName(), source, destination, sourceSmeId);
      return new RouteTrace(trace.getRouteId(), trace.getTrace());

    } finally {
      cfgFileManager.getFileSystem().delete(routesTracingConfig);
    }
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    if (!cc.isOnline())
      return null;
    ConfigState state = cc.getRoutesState();
    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    if (state != null) {
      long lastUpdate = cfgFileManager.getLastModified();
      for (Map.Entry<Integer, Long> e : state.getInstancesUpdateTimes().entrySet()) {
        SmscConfigurationStatus s = e.getValue() >= lastUpdate ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
        result.put(e.getKey(), s);
      }
    }
    return result;
  }
}
