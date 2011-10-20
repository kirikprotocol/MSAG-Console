package ru.sibinco.smsx.stats;

import java.util.Date;

/**
 * User: artem
 * Date: 20.10.11
 */
public interface RecordsProvider {

  void processRecords(Date date, RecordHandler handler) throws Exception;
}
