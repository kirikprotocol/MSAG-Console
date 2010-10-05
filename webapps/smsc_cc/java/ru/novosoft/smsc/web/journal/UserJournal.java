package ru.novosoft.smsc.web.journal;

/**
 * @author Artem Snopkov
 */
public class UserJournal {
  
  private final Journal j;
  private final String user;
  
  UserJournal(String user, Journal j) {
    this.user = user;
    this.j = j;
  } 
  
  private ActionJournal addRecord(JournalRecord.Type type, String description, String ... args) {
    return new ActionJournal(j, type, user, description, args);
  }

  public ActionJournal add(String ... args) {
    return addRecord(JournalRecord.Type.ADD, null, args);
  }

  public ActionJournal remove(String ... args) {
    return addRecord(JournalRecord.Type.REMOVE, null, args);
  }

  public ActionJournal change(String description, String ... args) {
    return addRecord(JournalRecord.Type.CHANGE, description, args);
  }

  public ActionJournal start(String ... args) {
    return addRecord(JournalRecord.Type.START, null, args);
  }

  public ActionJournal stop(String ... args) {
    return addRecord(JournalRecord.Type.STOP, null, args);
  }

  public ActionJournal disconnect(String ... args) {
    return addRecord(JournalRecord.Type.DISCONNECT, null, args);
  }

  public ActionJournal switchTo(String ... args) {
    return addRecord(JournalRecord.Type.SWITCH, null, args);
  }
}
