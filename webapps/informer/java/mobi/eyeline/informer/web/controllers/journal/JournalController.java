package mobi.eyeline.informer.web.controllers.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.WebContext;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.controllers.InformerController;
import mobi.eyeline.informer.web.journal.Journal;
import mobi.eyeline.informer.web.journal.JournalFilter;
import mobi.eyeline.informer.web.journal.JournalRecord;
import mobi.eyeline.informer.web.users.User;
import mobi.eyeline.informer.web.users.UsersSettings;

import javax.faces.model.SelectItem;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class JournalController extends InformerController {

  private final Journal journal;
  private final UsersSettings users;

  private String filterByUser;
  private String filterBySubject;
  private Date filterByStartDate;
  private Date filterByEndDate;


  public JournalController() throws AdminException {
    this.journal = WebContext.getInstance().getJournal();
    this.users = WebContext.getInstance().getConfiguration().getUserSettings();
    clearFilter();
  }

  public void clearFilter() {
    filterByUser = null;
    filterBySubject = null;
    filterByStartDate = null;
    filterByEndDate = null;
  }

  public String getFilterByUser() {
    return filterByUser;
  }

  public void setFilterByUser(String filterByUser) {
    this.filterByUser = filterByUser;
    if (this.filterByUser != null) {
      this.filterByUser = this.filterByUser.trim();
      if (this.filterByUser.length() == 0)
        this.filterByUser = null;
    }
  }

  public String getFilterBySubject() {
    return filterBySubject;
  }

  public void setFilterBySubject(String filterBySubject) {
    this.filterBySubject = filterBySubject;
    if (this.filterBySubject != null) {
      this.filterBySubject = this.filterBySubject.trim();
      if (this.filterBySubject.length() == 0)
        this.filterBySubject = null;
    }
  }

  public List<SelectItem> getUniqueSubjectNamesFromJournal() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    result.add(new SelectItem(null));
    for (Map.Entry<String,String> e : journal.getSubjects(getLocale()).entrySet())
      result.add(new SelectItem(e.getKey(), e.getValue()));

    return result;
  }

  public Date getFilterByStartDate() {
    return filterByStartDate;
  }

  public void setFilterByStartDate(Date filterByStartDate) {
    this.filterByStartDate = filterByStartDate;
  }

  public Date getFilterByEndDate() {
    return filterByEndDate;
  }

  public void setFilterByEndDate(Date filterByEndDate) {
    this.filterByEndDate = filterByEndDate;
  }

  public DataTableModel getRecords() {
    return new DataTableModel() {

      private List<JournalRecord> records = null;

      private List<JournalRecord> loadRecord() throws AdminException{
        if(records == null) {
          records = journal.getRecords(new JournalFilter().
              setStartDate(filterByStartDate).setEndDate(filterByEndDate).setUser(filterByUser).
              setSubjectKey(filterBySubject));
        }
        return records;
      }

    public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
      List<JournalRecord> recs;
      try{
        recs = loadRecord();
      }catch (AdminException e){
        addError(e);
        recs = Collections.emptyList();
      }

      // Сортируем записи
      if (sortOrder != null) {
        final int mul = sortOrder.isAsc() ? 1 : -1;
        Collections.sort(recs, new Comparator<JournalRecord>() {
          public int compare(JournalRecord o1, JournalRecord o2) {
            if (sortOrder.getColumnId().equals("user")) {
              return mul*o1.getUser().compareTo(o2.getUser());
            } else if (sortOrder.getColumnId().equals("subject")) {
              Locale l = getLocale();
              return mul*o1.getSubject(l).compareTo(o2.getSubject(l));
            } else if (sortOrder.getColumnId().equals("time")) {
              return o1.getTime() >= o2.getTime() ? mul : -mul;
            }
            return 0;
          }
        });
      }

      List<JournalTableRow> result = new ArrayList<JournalTableRow>(recs.size());
      Locale l = getLocale();
      for (int i=startPos; i < Math.min(recs.size(), startPos + count); i++) {
        JournalRecord r = recs.get(i);
        JournalTableRow row = new JournalTableRow();
        row.setDate(new Date(r.getTime()));
        row.setUser(r.getUser());
        row.setSubject(r.getSubject(l));
        row.setDescription(r.getDescription(l));

        User u = users.getUser(r.getUser());
        row.setUserDetails(u.getLastName() + " " + u.getFirstName() + " (" + u.getDept() + ")");

        result.add(row);
      }

      return result;
    }

    public int getRowsCount() {
      List<JournalRecord> recs;
      try{
        recs = loadRecord();
      }catch (AdminException e){
        addError(e);
        recs = Collections.emptyList();
      }
      return recs.size();
    }
    };
  }




  public class JournalTableRow {
    private Date date;
    private String user;
    private String userDetails;
    private String subject;
    private String description;

    public Date getDate() {
      return date;
    }

    public void setDate(Date date) {
      this.date = date;
    }

    public String getUser() {
      return user;
    }

    public void setUser(String user) {
      this.user = user;
    }

    public String getUserDetails() {
      return userDetails;
    }

    public void setUserDetails(String userDetails) {
      this.userDetails = userDetails;
    }

    public String getSubject() {
      return subject;
    }

    public void setSubject(String subject) {
      this.subject = subject;
    }

    public String getDescription() {
      return description;
    }

    public void setDescription(String description) {
      this.description = description;
    }
  }

}
