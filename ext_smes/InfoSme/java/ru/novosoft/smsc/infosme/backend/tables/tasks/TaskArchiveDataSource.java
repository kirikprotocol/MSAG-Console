package ru.novosoft.smsc.infosme.backend.tables.tasks;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.InfoSme;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSource;

import java.io.File;
import java.io.FileFilter;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * alkhal: alkhal
 */
public class TaskArchiveDataSource extends AbstractDataSource{

  private Category logger = Category.getInstance(this.getClass());

  private final InfoSme infoSme;

  private final String archiveDir;

  public TaskArchiveDataSource(InfoSme infoSme, String archiveDir) {
    super(new String[]{"id", "name", "provider", "enabled", "priority", "retryOnFail", "replaceMessage", "svcType", "generating", "processing", "trackIntegrity", "startDate", "endDate", "owner", "delivery"});
    this.infoSme = infoSme;
    this.archiveDir = archiveDir;
  }

  public Task get(Date archiveDate, String id) throws AdminException{
   final Task[] task = new Task[1];
    Visitor visitor = new Visitor() {
      public boolean visit(TaskDataItem item, Task t) {
        task[0]=t;
        return false;
      }
    };
    TaskFilter f = new TaskFilter();
    f.setId(id);
    f.setArchiveDate(archiveDate);
    visit(visitor, f,  null);
    return task[0];
  }


  public void visit(Visitor visitor, Filter filter, TaskQuery.Progress progress) throws AdminException {

    try{
      Collection taskDirs = getTaskDirectories(new File(archiveDir), (TaskFilter)filter);

      int total = taskDirs.size();

      int processed = 0;

      for (Iterator iter = taskDirs.iterator(); iter.hasNext();) {
        try{
          File taskDir = (File)iter.next();

          Task t = new Task(null, taskDir.getName(), taskDir.getParentFile().getAbsolutePath(), null);
          TaskDataItem item = new TaskDataItem(t.getId(), t.getName(), t.getProvider(), t.isEnabled(), t.getPriority(), t.getRetryPolicy(), t.isReplaceMessage(),
              t.getSvcType(), false, false, t.isTrackIntegrity(), t.getStartDate(), t.getEndDate(), t.getOwner(), t.isDelivery());
          if (filter.isItemAllowed(item) && !visitor.visit(item, t)) {
            if(progress != null) {
              progress.setProgress(100);
            }
            break;
          }

          processed++;
          if(progress != null) {
            progress.setProgress(processed*100/total);
          }
        }catch (Exception e) {
          logger.error(e,e);
          e.printStackTrace();
        }
      }
      if(progress != null) {
        progress.setProgress(100);
      }
    }catch (Exception e) {
      throw new AdminException(e.getMessage(), e);
    }
  }

  public QueryResultSet query(Query query_to_run)
  {
    init(query_to_run);
    TaskQuery query = (TaskQuery)query_to_run;
    Set generatingTasks = new HashSet();
    Set processingTasks = new HashSet();
    try {
      if (infoSme.getInfo().isOnline()) {
        generatingTasks.addAll(infoSme.getGeneratingTasks());
        processingTasks.addAll(infoSme.getProcessingTasks());
      }

      Collection taskDirs = getTaskDirectories(new File(archiveDir), (TaskFilter)query.getFilter());

      int total = taskDirs.size();

      int processed = 0;


      for (Iterator iter = taskDirs.iterator(); iter.hasNext();) {

        File taskDir = (File)iter.next();

        Task t = new Task(null, taskDir.getName(), taskDir.getParentFile().getAbsolutePath(), null);
        TaskDataItem item = new TaskDataItem(t.getId(), t.getName(), t.getProvider(), t.isEnabled(), t.getPriority(), t.getRetryPolicy(), t.isReplaceMessage(),
            t.getSvcType(), generatingTasks.contains(t.getId()), processingTasks.contains(t.getId()), t.isTrackIntegrity(), t.getStartDate(), t.getEndDate(), t.getOwner(), t.isDelivery());
        add(item);
        processed++;
        if(query.getProgress() != null) {
          query.getProgress().setProgress(processed*100/total);
        }

      }
      if(query.getProgress() != null) {
        query.getProgress().setProgress(100);
      }
    } catch (Exception e) {
      logger.error("Could not get tasks", e);
    }

    return getResults();
  }


  private static Collection getTaskDirectories(File archiveDir, final TaskFilter filter) throws ParseException {

    class DirFilter implements FileFilter{

      final SimpleDateFormat sdf;
      final Date fromDir;

      DirFilter(String format) throws ParseException{
        sdf =new SimpleDateFormat(format);
        fromDir = filter.getStartDate() == null ? null : sdf.parse(sdf.format(filter.getStartDate()));
      }

      public boolean accept(File file) {
        if (!file.isDirectory())
          return false;
        try {
          Date dirDate = sdf.parse(file.getName());
          return fromDir == null || dirDate.compareTo(fromDir) >= 0;
        } catch (ParseException e) {
          return false;
        }
      }
    }

    Collection byDay = new LinkedList();
    Collection result = new TreeSet();
    
    if(filter.getArchiveDate() != null) {
      SimpleDateFormat format = new SimpleDateFormat(
          new StringBuffer().append("yyyy").append(File.separatorChar).append("MM").
              append(File.separatorChar).append("dd").toString()
      );
      File f = new File(archiveDir, format.format(filter.getArchiveDate()));
      if(f.exists()) {
        byDay.add(f);
      }
    }else {
      Collection byMonth = new LinkedList();
      {
        File[] byYears = archiveDir.listFiles(new DirFilter("yyyy"));
        for(int i=0; i<byYears.length; i++) {
          File dir = byYears[i];
          File[] byM = dir.listFiles(new DirFilter("MM"));
          byMonth.addAll(Arrays.asList(byM));
        }
      }
      for(Iterator i = byMonth.iterator(); i.hasNext();) {
        File dir = (File)i.next();
        File[] byD = dir.listFiles(new DirFilter("dd"));
        byDay.addAll(Arrays.asList(byD));
      }
    }

    for(Iterator i = byDay.iterator(); i.hasNext();) {
      File dir = (File)i.next();
      if(dir.isDirectory()) {
        File[] files = dir.listFiles();
        for(int j=0; j<files.length;j++) {
          final File f = files[j];
          if(f.isDirectory()) {
            if(filter.getId() != null && filter.getId().equals(f.getName())) {
              return new ArrayList(1){{add(f);}};
            }
            result.add(f);
          }
        }
      }
    }
    return result;
  }

  public interface Visitor {

    public boolean visit(TaskDataItem item, Task t);

  }

}
