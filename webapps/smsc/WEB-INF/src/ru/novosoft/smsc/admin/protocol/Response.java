// Decompiled by Jad v1.5.5.3. Copyright 1997-98 Pavel Kouznetsov.
// Jad home page:      http://web.unicom.com.cy/~kpd/jad.html
// Decompiler options: packimports(3) printdflt
// Source File Name:   Response.java

package ru.novosoft.smsc.admin.protocol;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import javax.xml.parsers.*;
import org.apache.log4j.Category;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.xml.Utils;
import ru.novosoft.smsc.util.StringEncoderDecoder;

// Referenced classes of package ru.novosoft.smsc.admin.protocol:
//      ResponseEntityResolver, Status

public class Response
{

  public Response(byte text[])
    throws AdminException
  {
    doc = null;
    status = 0;
    logger = Category.getInstance("smsc.admin.service.protocol.Response");
    try
    {
      DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
      factory.setIgnoringComments(true);
      factory.setValidating(false);
      DocumentBuilder builder = factory.newDocumentBuilder();
      logger.debug("setting entity resolver for response");
      builder.setEntityResolver(new ResponseEntityResolver());
      ByteArrayInputStream inputStream = new ByteArrayInputStream(text);
      InputSource source = new InputSource(inputStream);
      doc = builder.parse(source);
      Element main = doc.getDocumentElement();
      status = parseStatus(main.getAttribute("status"));
    }
    catch(FactoryConfigurationError error)
    {
      logger.warn("Unrecognized error in constructor", error);
      throw new AdminException(error.getMessage());
    }
    catch(ParserConfigurationException e)
    {
      logger.warn("Unrecognized error in constructor", e);
      throw new AdminException(e.getMessage());
    }
    catch(SAXException e)
    {
      logger.warn("Unrecognized error in constructor", e);
      throw new AdminException(e.getMessage());
    }
    catch(IOException e)
    {
      logger.warn("Unrecognized error in constructor", e);
      throw new AdminException(e.getMessage());
    }
    catch(Throwable t)
    {
      logger.warn("Unrecognized error in constructor", t);
      throw new AdminException(t.getMessage());
    }
  }

  protected String getStatusString()
  {
    for(int i = 0; i < statuses.length; i++)
      if(statuses[i].status == status)
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

  private byte parseStatus(String status)
  {
    for(int i = 0; i < statuses.length; i++)
      if(statuses[i].name.equalsIgnoreCase(status))
        return statuses[i].status;

    logger.warn("Unkonwn status \"" + status + "\" in response");
    return 0;
  }

  public static final byte StatusUnknown = 0;
  public static final byte StatusError = 1;
  public static final byte StatusOk = 2;
  public static final Status statuses[] = {
    new Status("Undefined", (byte)0), new Status("Error", (byte)1), new Status("Ok", (byte)2)
  };
  private Document doc = null;
  private byte status = 0;
  private Category logger = null;

}
