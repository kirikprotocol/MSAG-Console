package ru.novosoft.smsc.infosme.backend.tables.stat;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

import java.util.Date;

/**
 * User: artem
 * Date: 02.06.2008
 */

public class StatisticDataItem extends AbstractDataItem {
  public StatisticDataItem(Date period, String taskId, String taskName, Integer generated, Integer delivered, Integer retried, Integer failed) {
    values.put("period", period);
    values.put("taskId", taskId);
    values.put("taskName", taskName);
    values.put("generated", generated);
    values.put("delivered", delivered);
    values.put("retried", retried);
    values.put("failed", failed);
  }
}
