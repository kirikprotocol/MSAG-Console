package ru.sibinco.mci.profile;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 09.09.2004
 * Time: 18:39:26
 * To change this template use File | Settings | File Templates.
 */
public class FormatType
{
  public final static long DEFAULT_TYPE = -1;
      
  private long id = DEFAULT_TYPE;
  private String name = null;

  public FormatType() {
  }
  public FormatType(long id, String name) {
    this.id = id; this.name = name;
  }

  public long getId() {
    return id;
  }
  public String getName() {
    return name;
  }
}
