package ru.novosoft.smsc.infosme.backend.config;

/**
 * User: artem
 * Date: 27.01.2009
 */
public class ConfigChanges {
  private final Changes tasksChanges;
  private final Changes schedulesChanges;
  private final Changes policiesChanges;

  public ConfigChanges(Changes schedulesChanges, Changes tasksChanges, Changes policiesChanges) {
    this.schedulesChanges = schedulesChanges;
    this.tasksChanges = tasksChanges;
    this.policiesChanges = policiesChanges;
  }

  public Changes getSchedulesChanges() {
    return schedulesChanges;
  }

  public Changes getTasksChanges() {
    return tasksChanges;
  }

  public Changes getPoliciesChanges() {
    return policiesChanges;
  }
}
