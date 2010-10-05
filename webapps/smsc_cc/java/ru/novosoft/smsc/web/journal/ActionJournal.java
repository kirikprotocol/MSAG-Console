package ru.novosoft.smsc.web.journal;

/**
 * @author Artem Snopkov
 */
public class ActionJournal {

  private final Journal j;
  private final JournalRecord.Type type;
  private final String user;
  private final String[] args;
  private final String description;

  ActionJournal(Journal j, JournalRecord.Type type, String user, String description, String... args) {
    this.j = j;
    this.type = type;
    this.user = user;
    this.description = description;
    this.args = args;
  }

  private void addRecord(JournalRecord.Subject subject, String... args) {
    if (this.description != null)
      addRecord(subject, subject.getKey() + "." + description, args);
    else
      addRecord(subject, subject.getKey() + "." + type.name().toLowerCase(), args);    
  }

  private void addRecord(JournalRecord.Subject subject, String description, String... args) {
    JournalRecord r = j.addRecord(type, subject, user);
    int len = args.length + this.args.length;
    String[] allArgs = new String[len];
    if (this.args.length > 0)
      System.arraycopy(this.args, 0, allArgs, 0, this.args.length);
    if (args.length > 0)
      System.arraycopy(args, 0, allArgs, this.args.length, args.length);
    
    r.setDescription(description, allArgs);
  }

  public void smsc(String... args) {
    addRecord(JournalRecord.Subject.SMSC, args);
  }

  public void reschedule(String... args) {
    addRecord(JournalRecord.Subject.RESCHEDULE, args);
  }

  public void closedGroup(String... args) {
    addRecord(JournalRecord.Subject.CLOSED_GROUPS,  args);
  }

  public void user(String... args) {
    addRecord(JournalRecord.Subject.USERS, args);
  }

  public void mapLimit(String... args) {
    addRecord(JournalRecord.Subject.MAP_LIMIT, args);
  }

  public void logger(String... args) {
    addRecord(JournalRecord.Subject.LOGGING, args);
  }

  public void fraud(String... args) {
    addRecord(JournalRecord.Subject.FRAUD, args);
  }

  public void snmp(String... args) {
    addRecord(JournalRecord.Subject.SNMP, args);
  }

  public void sme(String... args) {
    addRecord(JournalRecord.Subject.SME, args);
  }

  public void acl(String... args) {
    addRecord(JournalRecord.Subject.ACL, args);
  }

  public void category(String... args) {
    addRecord(JournalRecord.Subject.CATEGORY, args);
  }

  public void alias(String... args) {
    addRecord(JournalRecord.Subject.ALIAS, args);
  }

  public void archiveDaemon(String... args) {
    addRecord(JournalRecord.Subject.ARCHIVE_DAEMON, args);
  }

  public void msc(String... args) {
    addRecord(JournalRecord.Subject.MSC, args);
  }

  public void profile(String... args) {
    addRecord(JournalRecord.Subject.PROFILE, args);
  }

  public void provider(String... args) {
    addRecord(JournalRecord.Subject.PROVIDER, args);
  }

  public void region(String... args) {
    addRecord(JournalRecord.Subject.REGION, args);
  }
}
