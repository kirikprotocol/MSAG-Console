package ru.novosoft.smsc.infosme.backend.tables.tasks;

import ru.novosoft.smsc.jsp.util.tables.EmptyFilter;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 1:19:58 PM
 */
public class TaskQuery extends AbstractQueryImpl {

  private Progress progress;

  public TaskQuery(int expectedResultsQuantity, String sortOrder, int startPosition) {
    super(expectedResultsQuantity, new EmptyFilter(), sortOrder, startPosition);
  }

  public TaskQuery(Filter filter, int expectedResultsQuantity, String sortOrder, int startPosition) {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }

  public void setProgress(Progress progress) {
    this.progress = progress;
  }

  public Progress getProgress() {
    return progress;
  }


  public static class Progress {

    private int progress = 0;

    public int getProgress() {
      return progress;
    }

    void setProgress(int progress) {
      System.out.println("Set progress to: "+progress);
      this.progress = progress;
    }
  }
}
