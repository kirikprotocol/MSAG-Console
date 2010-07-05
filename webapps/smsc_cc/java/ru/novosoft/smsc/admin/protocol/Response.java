package ru.novosoft.smsc.admin.protocol;

import org.apache.log4j.Category;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.xml.DtdsEntityResolver;
import ru.novosoft.smsc.util.XmlUtils;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;


public class Response {

  private static final Category logger = Category.getInstance("smsc.admin.services.protocol.Response");

  public static final byte StatusUnknown = 0;
  public static final byte StatusError = 1;
  public static final byte StatusOk = 2;
  public static final Status statuses[] = {
      new Status("Undefined", (byte) 0), new Status("Error", (byte) 1), new Status("Ok", (byte) 2)
  };

  private Document doc = null;
  private byte status = 0;


  public Response(byte text[]) throws AdminException {
    doc = null;
    status = 0;
    try {
      DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
      factory.setIgnoringComments(true);
      factory.setValidating(false);
      DocumentBuilder builder = factory.newDocumentBuilder();
      logger.debug("setting entity resolver for response");
      builder.setEntityResolver(new DtdsEntityResolver());
      //!!!
      String a = new String(text);
      Reader in = new StringReader(a);
      InputSource source = new InputSource(in);
      doc = builder.parse(source);
      Element main = doc.getDocumentElement();
      status = parseStatus(main.getAttribute("status"));
    }
    catch (FactoryConfigurationError error) {
      logger.warn("Unrecognized error in constructor", error);
      throw new AdminException(error.getMessage());
    }
    catch (ParserConfigurationException e) {
      logger.warn("Unrecognized error in constructor", e);
      throw new AdminException(e.getMessage());
    }
    catch (SAXException e) {
      logger.warn("Unrecognized error in constructor", e);
      throw new AdminException(e.getMessage());
    }
    catch (IOException e) {
      logger.warn("Unrecognized error in constructor", e);
      throw new AdminException(e.getMessage());
    }
    catch (Throwable t) {
      logger.warn("Unrecognized error in constructor", t);
      throw new AdminException(t.getMessage());
    }
  }

  protected String getStatusString() {
    for (int i = 0; i < statuses.length; i++)
      if (statuses[i].status == status)
        return statuses[i].name;

    return "unknown";
  }

  public String toString() {
    return "Response = " + getStatusString() + " [" + super.toString() + "]";
  }

  public Document getData() {
    return doc;
  }

  public String getDataAsString() {
    return XmlUtils.getNodeText(doc.getDocumentElement());
  }

  public byte getStatus() {
    return status;
  }

  private byte parseStatus(String status) {
    for (int i = 0; i < statuses.length; i++)
      if (statuses[i].name.equalsIgnoreCase(status))
        return statuses[i].status;

    logger.warn("Unknown status \"" + status + "\" in response");
    return 0;
  }



}
