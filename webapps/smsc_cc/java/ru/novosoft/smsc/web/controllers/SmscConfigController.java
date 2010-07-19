package ru.novosoft.smsc.web.controllers;

import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.smsc.CommonSettings;
import ru.novosoft.smsc.admin.smsc.InstanceSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.beans.CommonConfig;
import ru.novosoft.smsc.web.beans.InstanceConfig;

import javax.faces.event.ValueChangeEvent;
import javax.faces.model.SelectItem;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

/**
 * author: alkhal
 */
public class SmscConfigController implements Serializable {

  private int currentInstance;

  private AdminContext adminContext;

  private int instancesCount = 0;

  private boolean initialized;

  private CommonConfig commonConfig;

  private InstanceConfig[] instanceConfigs;

  public SmscConfigController() {
    adminContext = WebContext.getInstance().getAdminContext();
    if (!initialized) {
      instancesCount = adminContext.getSmscManager().getSmscInstancesCount();
      initCommonConfig();
      initInstances();
    }
    initialized = true;
  }

  private void initCommonConfig() {
    CommonSettings cs = adminContext.getSmscManager().getCommonSettings();
    commonConfig = new CommonConfig();
    //core                                                             todo others
    commonConfig.setState_machines_count(cs.getState_machines_count());
    commonConfig.setService_center_address(cs.getService_center_address());
    //profiler
    commonConfig.setProfilerSystemId(cs.getProfilerSystemId());
  }

  private void initInstances() {
    instanceConfigs = new InstanceConfig[instancesCount];
    for (int i = 0; i < instancesCount; i++) {
      InstanceSettings is = adminContext.getSmscManager().getInstanceSettings(i);
      instanceConfigs[i] = new InstanceConfig();
      instanceConfigs[i].setInstanceNumber(i);
      instanceConfigs[i].setAdminHost(is.getAdminHost());
      instanceConfigs[i].setAdminPort(is.getAgentPort());
      //todo others
      instanceConfigs[i].setAgentHost(is.getAgentHost());
      instanceConfigs[i].setAgentPort(is.getAgentPort());
    }
  }

  public boolean isInitialized() {
    return initialized;
  }

  public void setInitialized(boolean initialized) {
    this.initialized = initialized;
  }

  public int getInstancesCount() {
    return instancesCount;
  }

  public void setInstancesCount(int instancesCount) {
    this.instancesCount = instancesCount;
  }

  public List<SelectItem> getInstancesIndexes() {
    List<SelectItem> result = new ArrayList<SelectItem>(instancesCount);
    for (int i = 0; i < instancesCount; i++) {
      result.add(new SelectItem(i, Integer.toString(i)));
    }
    return result;
  }

  public int getCurrentInstance() {
    return currentInstance;
  }

  public InstanceConfig[] getInstanceConfigs() {
    return instanceConfigs;
  }

  public CommonConfig getCommonConfig() {
    return commonConfig;
  }

  public void setCurrentInstance(int currentInstance) {
    this.currentInstance = currentInstance;
  }

  public void valueChange(ValueChangeEvent event) {
    currentInstance = (Integer) event.getNewValue();
  }

  public String saveAll() {
    return null;
  }
}
