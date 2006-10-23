package ru.novosoft.smsc.jsp.smsc.snmp.tables;

/**
 * User: artem
 * Date: 17.10.2006
 */

public class AllStatsTableScheme extends StatsTableScheme{

  public static final AllStatsTableScheme SCHEME = new AllStatsTableScheme();

  public final String SUBMIT_TIME = addField("submit_time");
  public final String ALARM_ID = addField("alarm_id");
  public final String ALARM_CATEGORY = addField("alarm_category");
  public final String SEVERITY = addField("severity");
  public final String TEXT = addField("text");
}
