package ru.novosoft.smsc.admin.category;


/**
 * Created by igork Date: 22.03.2004 Time: 19:45:18
 */
public class Category
{
   public static final String[] columnNames = {
		"id", "name",
  };
  private long id;
  private String name;

  public Category(final long id, final String name)
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

  public void setName(final String name)
  {
    this.name = name;
  }

  public String toString()
  {
    return "Category{" +
           "id=" + id +
           ", name='" + name + "'" +
           "}";
  }
}
