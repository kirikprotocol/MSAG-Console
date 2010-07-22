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
      throw new ProtocolException("invalid_response", error);
    }
    catch (ParserConfigurationException e) {
      logger.warn("Unrecognized error in constructor", e);
      throw new ProtocolException("invalid_response", e);
    }
    catch (SAXException e) {
      logger.warn("Unrecognized error in constructor", e);
      throw new ProtocolException("invalid_response", e);
    }
    catch (IOException e) {
      logger.warn("Unrecognized error in constructor", e);
      throw new ProtocolException("invalid_response", e);
    }
    catch (Throwable t) {
      logger.warn("Unrecognized error in constructor", t);
      throw new ProtocolException("invalid_response", t);
    }
  }

  protected String getStatusString() {
    for (Status statuse : statuses)
      if (statuse.status == status)
        return statuse.name;

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
    for (Status statuse : statuses)
      if (statuse.name.equalsIgnoreCase(status))
        return statuse.status;

    logger.warn("Unknown status \"" + status + "\" in response");
    return 0;
  }



}
