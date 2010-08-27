package ru.novosoft.smsc.web.config.changelog;

import java.util.Collection;

/**
 * @author Artem Snopkov
 */
public interface ChangeLog {

  Collection<ChangeLogRecord> getRecords();

}
