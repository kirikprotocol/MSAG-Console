package ru.novosoft.smsc.web.config.closed_groups;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroup;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.journal.Journal;
import ru.novosoft.smsc.web.journal.JournalRecord;

import java.util.Collection;

/**
 * Журналируемая закрытая группа
 * @author Aleksander Khalitov
 */

class WClosedGroup implements ClosedGroup{

  private ClosedGroup group;

  private Journal journal;
  
  private String user;

  WClosedGroup(ClosedGroup group, Journal journal, String user) {
    if(group == null || journal == null) {
      throw new IllegalArgumentException("Some arguments are null: group="+group+" journal="+journal);

    }
    this.group = group;
    this.journal = journal;
    this.user = user;
  }

  public int getId() throws AdminException {
    return group.getId();
  }

  public String getName() throws AdminException {
    return group.getName();
  }

  public String getDescription() throws AdminException {
    return group.getDescription();
  }

  public void setDescription(String description) throws AdminException {
    String old = getDescription();
    group.setDescription(description);
     JournalRecord r = journal.addRecord(JournalRecord.Type.CHANGE, JournalRecord.Subject.CLOSED_GROUPS, user);
    r.setDescription("closed_group_change_description", old, description, getName());
  }

  public Collection<Address> getMasks() throws AdminException {
    return group.getMasks();
  }

  public void removeMask(Address mask) throws AdminException {
    group.removeMask(mask);
    JournalRecord r = journal.addRecord(JournalRecord.Type.CHANGE, JournalRecord.Subject.CLOSED_GROUPS, user);
    r.setDescription("closed_group_remove_mask", mask.getSimpleAddress(), getName());
  }

  public void addMask(Address mask) throws AdminException {
    group.addMask(mask);
    JournalRecord r = journal.addRecord(JournalRecord.Type.CHANGE, JournalRecord.Subject.CLOSED_GROUPS, user);
    r.setDescription("closed_group_add_mask", mask.getSimpleAddress(), getName());
  }



}