package ru.sibinco.lib.backend.protocol;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import ru.sibinco.lib.backend.util.xml.Utils;


/**
 * Created by igork
 * Date: 25.05.2004
 * Time: 16:15:49
 */
public class Response
{
  static class Status
  {
    Status(String name, byte status)
    {
      this.status = status;
      this.name = name;
    }

    byte status = 0;
    String name = null;
  }


  public static final byte StatusUnknown = 0;
  public static final byte StatusError = 1;
  public static final byte StatusOk = 2;

  private static final Status statuses[] = {
    new Status("Undefined", StatusUnknown),
    new Status("Error", StatusError),
    new Status("Ok", StatusOk)
  };

  protected final Document doc;
  private final byte status;
  private final Logger logger = Logger.getLogger(this.getClass());

  public Response(Document doc)
  {
    this.doc = doc;
    Element main = doc.getDocumentElement();
    status = parseStatus(main.getAttribute("status"));
  }

  private byte parseStatus(String status)
  {
    for (int i = 0; i < statuses.length; i++)
      if (statuses[i].name.equalsIgnoreCase(status))
        return statuses[i].status;

    logger.warn("Unkonwn status \"" + status + "\" in response");
    return 0;
  }

  protected String getStatusString()
  {
    for (int i = 0; i < statuses.length; i++)
      if (statuses[i].status == status)
        return statuses[i].name;

    return "unknown";
  }

  public String toString()
  {
    return "Response = " + getStatusString() + " [" + super.toString() + "]";
  }

  public Document getData()
  {
    return doc;
  }

  public String getDataAsString()
  {
    return Utils.getNodeText(doc.getDocumentElement());
  }

  public byte getStatus()
  {
    return status;
  }
}
