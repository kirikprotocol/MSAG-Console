package ru.sibinco.smppgw.backend.sme;


/**
 * Created by igork
 * Date: 22.03.2004
 * Time: 19:45:18
 */
public class Provider
{
  private long id;
  private String name;

  public Provider(long id, String name)
  {
    this.id = id;
    this.name = name;
  }

  public long getId()
  {
    return id;
  }

  public String getName()
  {
    return name;
  }

  public void setName(String name)
  {
    this.name = name;
  }
}
