package ru.novosoft.smsc.infosme.backend.tables.tasks;

import ru.novosoft.smsc.infosme.backend.config.tasks.Task;

/**
 * User: artem
 * Date: 20.04.2009
 */
public interface TaskVisitor {

  public boolean visit(TaskDataItem t);
}
