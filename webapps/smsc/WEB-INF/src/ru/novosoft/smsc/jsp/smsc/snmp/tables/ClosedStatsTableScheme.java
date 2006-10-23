package ru.novosoft.smsc.jsp.smsc.snmp.tables;

/**
 * User: artem
 * Date: 17.10.2006
 */

public class ClosedStatsTableScheme extends AllStatsTableScheme {
  public static final ClosedStatsTableScheme SCHEME = new ClosedStatsTableScheme();

//  public final String START_DATE = addField("start_date");
//  public final String END_DATE = addField("end_date");
//  public final String ALARM_ID = addField("alarm_id");
//  public final String ALARM_CATEGORY = addField("alarm_category");
//  public final String START_SEVERITY = addField("start_severity");
//  public final String CLOSE_SEVERITY = addField("close_severity");
//  public final String START_TEXT = addField("start_text");
//  public final String CLOSE_TEXT = addField("close_text");
    public final String CLOSE_FLAG = addField("close_flag");
}
