/*
 * Created by igork
 * Date: Feb 28, 2002
 * Time: 12:49:06 PM
 */
package ru.novosoft.smsc.admin.service.protocol;

import org.apache.log4j.Category;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.ByteArrayInputStream;
import java.io.IOException;

import ru.novosoft.smsc.admin.AdminException;

class Status
{
  byte status = 0;
  String name = "Undefined";

  Status(String name, byte status)
  {
    this.status = status;
    this.name = name;
  }
}

;

public class Response
{
  public static final byte StatusUnknown = 0;
  public static final byte StatusError = 1;
  public static final byte StatusOk = 2;
  public static final Status statuses[] = {new Status("Undefined", StatusUnknown),
                                           new Status("Error", StatusError),
                                           new Status("Ok", StatusOk)};

  public Response(byte text[])
          throws AdminException
  {
    try {
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
    } catch (FactoryConfigurationError error) {
      logger.warn("Unrecognized error in constructor", error);
      throw new AdminException(error.getMessage());
    } catch (ParserConfigurationException e) {
      logger.warn("Unrecognized error in constructor", e);
      throw new AdminException(e.getMessage());
    } catch (SAXException e) {
      logger.warn("Unrecognized error in constructor", e);
      throw new AdminException(e.getMessage());
    } catch (IOException e) {
      logger.warn("Unrecognized error in constructor", e);
      throw new AdminException(e.getMessage());
    } catch (Throwable t) {
      logger.warn("Unrecognized error in constructor", t);
      throw new AdminException(t.getMessage());
    }
  }

  protected String getStatusString()
  {
    for (int i = 0; i < statuses.length; i++) {
      if (statuses[i].status == status) {
        return statuses[i].name;
      }
    }
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

  public byte getStatus()
  {
    return status;
  }

  private Document doc = null;
  private byte status = StatusUnknown;
  private Category logger = Category.getInstance("smsc.admin.service.protocol.Response");

  private byte parseStatus(String status)
  {
    for (int i = 0; i < statuses.length; i++) {
      if (statuses[i].name.equalsIgnoreCase(status))
        return statuses[i].status;
    }
    logger.warn("Unkonwn status \"" + status + "\" in response");
    return StatusUnknown;
  }
}
