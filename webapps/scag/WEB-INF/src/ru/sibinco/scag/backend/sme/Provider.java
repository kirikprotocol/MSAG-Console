package ru.sibinco.scag.backend.sme;

import ru.sibinco.lib.backend.util.StringEncoderDecoder;


/**
 * Created by igork Date: 22.03.2004 Time: 19:45:18
 */
public class Provider
{
  private long id;
  private String name;

  public Provider(final long id, final String name)
  {
    this.id = id;
    this.name = StringEncoderDecoder.encode(name);
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
