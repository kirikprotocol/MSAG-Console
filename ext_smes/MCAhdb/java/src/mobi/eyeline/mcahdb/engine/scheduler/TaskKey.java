package mobi.eyeline.mcahdb.engine.scheduler;

/**
 * User: artem
* Date: 29.08.2008
*/
class TaskKey {
  private final String caller;
  private final String called;

  public TaskKey(String caller, String called) {
    this.caller = caller;
    this.called = called;
  }

  public int hashCode() {
    return caller.hashCode() + called.hashCode();
  }

  public boolean equals(Object o) {
    TaskKey k = (TaskKey)o;
    return k.caller.equals(caller) && k.called.equals(called);
  }
}
