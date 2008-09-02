package mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file.page;

/**
 * User: artem
 * Date: 29.08.2008
 */

public class TaskPointer implements Comparable {
  private final String pageId;
  private final long pointer;

  public TaskPointer(String pageId, long pointer) {
    this.pageId = pageId;
    this.pointer = pointer;
  }

  public String getPageId() {
    return pageId;
  }

  public long getPointer() {
    return pointer;
  }


  public int compareTo(Object o) {
    return pageId.compareTo(((TaskPointer)o).pageId);
  }
}
