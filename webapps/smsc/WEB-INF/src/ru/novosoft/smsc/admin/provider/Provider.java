package ru.novosoft.smsc.admin.provider;


/**
 * Created by igork Date: 22.03.2004 Time: 19:45:18
 */
public class Provider
{
  public static final String[] columnNames = {
		"id", "name",
  };
  private long id;
  private String name;

  public Provider(final long id, final String name)
  {
    this.id = id;
    this.name = name;
  }

   public Provider( final String name)
  {
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

  public void setName(final String name)
  {
    this.name = name;
  }

  public String toString()
  {
    return "Provider{" +
           "id=" + id +
           ", name='" + name + "'" +
           "}";
  }
}
