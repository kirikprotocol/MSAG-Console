package mobi.eyeline.informer.web.controllers.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.journal.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.model.SelectItem;
import java.io.IOException;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Контроллер для просмотра журнала
 *
 * @author Aleksandr Khalitov
 */
public class JournalController extends InformerController {

  private final Journal journal;


  private String filterByUser;
  private String filterBySubject;
  private Date filterByStartDate;
  private Date filterByEndDate;

  private boolean init = false;

  private List<JournalRecord> records;

  public JournalController() throws AdminException {
    this.journal = getConfig().getJournal();
    Calendar c = Calendar.getInstance();
    c.set(Calendar.HOUR,0);
    c.set(Calendar.MINUTE,0);
    c.set(Calendar.SECOND,0);
    c.set(Calendar.MILLISECOND,0);
    c.add(Calendar.DATE,-3);
    filterByStartDate = c.getTime();
    init = true;
  }

  public void clearFilter() {
    filterByUser = null;
    filterBySubject = null;
    filterByStartDate = null;
    filterByEndDate = null;
    init = false;
  }

  public void query() {
    records = null;
    init = true;
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


  public List<SelectItem> getUniqueUsers() {
    List<SelectItem> ss = new LinkedList<SelectItem>();
    ss.add(new SelectItem("",""));
    for(User u : getConfig().getUsers()) {
      ss.add(new SelectItem(u.getLogin(), u.getLogin()));
    }
    return ss;
  }

  public List<SelectItem> getUniqueSubjectNamesFromJournal() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    result.add(new SelectItem(null));
    for (Subject s : journal.getSubjects()) {
      if (s == Subject.FTPSERVER && !getConfig().isFtpServerDeployed())
        continue;
      result.add(new SelectItem(s.getKey(), s.getSubject(getLocale())));
    }
    return result;
  }

  public boolean isInit() {
    return init;
  }

  public void setInit(boolean init) {
    this.init = init;
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

  private void loadRecord() throws AdminException {
    if (this.records == null) {
      final LinkedList<JournalRecord> records = new LinkedList<JournalRecord>();
      journal.getRecords(new JournalFilter().
          setStartDate(filterByStartDate).setEndDate(filterByEndDate).setUser(filterByUser).
          setSubject(Subject.getByKey(filterBySubject)),
          new JournalVisitor() {
            private static final int LIMIT = 1000;
            public boolean visit(JournalRecord r) {
              if (records.size() == LIMIT) {
                records.removeFirst();
              }
              records.addLast(r);
              return true;
            }
          });
      this.records = records;
    }
  }

  public DataTableModel getRecords() {
    if (!init) {
      records = Collections.emptyList();
    } else {
      try {
        loadRecord();
      } catch (AdminException e) {
        addError(e);
        records = Collections.emptyList();
      }
    }

    return new DataTableModel() {

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        // Сортируем записи
        final int mul = sortOrder == null ? -1 : sortOrder.isAsc() ? 1 : -1;
        Collections.sort(records, new Comparator<JournalRecord>() {
          public int compare(JournalRecord o1, JournalRecord o2) {
            if (sortOrder == null || sortOrder.getColumnId().equals("time")) {
              return o1.getTime() >= o2.getTime() ? mul : -mul;
            }else if (sortOrder.getColumnId().equals("user")) {
              return mul * o1.getUser().compareTo(o2.getUser());
            } else if (sortOrder.getColumnId().equals("subject")) {
              Locale l = getLocale();
              return mul * o1.getSubject().getSubject(l).compareTo(o2.getSubject().getSubject(l));
            }
            return 0;
          }
        });


        List<JournalTableRow> result = new ArrayList<JournalTableRow>(records.size());
        Locale l = getLocale();
        for (int i = startPos; i < Math.min(records.size(), startPos + count); i++) {
          JournalRecord r = records.get(i);
          JournalTableRow row = new JournalTableRow();
          row.setDate(new Date(r.getTime()));
          row.setUser(r.getUser());
          row.setSubject(r.getSubject().getSubject(l));
          row.setDescription(r.getDescription(l));

          User u = getConfig().getUser(r.getUser());
          row.setUserDetails(u != null ? u.getLastName() + " " + u.getFirstName() : r.getUser());
          result.add(row);
        }

        return result;
      }

      @Override
      public String getId(Object value) {
        return null;
      }

      public int getRowsCount() {
        return records.size();
      }
    };
  }

  @Override
  protected void _download(PrintWriter writer) throws IOException {
    try {
      loadRecord();
    } catch (AdminException e) {
      addError(e);
      records = Collections.emptyList();
    }
    super._download(writer);
    SimpleDateFormat sdf = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
    Locale locale = getLocale();
    for (JournalRecord r : records) {
      writer.println(StringEncoderDecoder.toCSVString(';',
          r.getUser(), sdf.format(new Date(r.getTime())),
          r.getSubject() == null ? "" : r.getSubject().getSubject(locale),
          r.getType(), r.getDescription(locale)));
    }
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
