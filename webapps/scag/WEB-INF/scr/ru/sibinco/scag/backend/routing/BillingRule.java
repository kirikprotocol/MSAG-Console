package ru.sibinco.smppgw.backend.routing;


/**
 * Created by igork Date: 21.09.2004 Time: 18:15:21
 */
public class BillingRule
{
  private final String id;
  private String text;

  public BillingRule(final String name, final String text)
  {
    this.id = name;
    this.text = text;
  }

  public String getId()
  {
    return id;
  }

  public String getText()
  {
    return text;
  }

  public void setText(String text)
  {
    this.text = text;
  }
}
