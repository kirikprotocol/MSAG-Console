package ru.novosoft.smsc.infosme.backend.config;

/**
 * User: artem
 * Date: 27.01.2009
 */
public class ConfigChanges {
  private final Changes tasksChanges;
  private final Changes schedulesChanges;

  public ConfigChanges(Changes schedulesChanges, Changes tasksChanges) {
    this.schedulesChanges = schedulesChanges;
    this.tasksChanges = tasksChanges;
  }

  public Changes getSchedulesChanges() {
    return schedulesChanges;
  }

  public Changes getTasksChanges() {
    return tasksChanges;
  }
}
