package mobi.eyeline.smsquiz;

import org.jdom.input.SAXBuilder;
import org.jdom.Document;
import org.jdom.Element;
import org.apache.log4j.Category;

import java.util.Date;
import java.text.SimpleDateFormat;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.IOException;

/**
 * author: alkhal
 * Date: 07.11.2008
 */
public class QuizBuilder {

  private static Category logger = Category.getInstance(QuizBuilder.class);

  private static String datePattern = "dd.MM.yyyy HH:mm";

  public static QuizXmlData parseQuiz(String path) {
    String address = null;
    Date dateBegin = null;
    Date dateEnd = null;
    SimpleDateFormat dateFormat = new SimpleDateFormat(datePattern);

    SAXBuilder sb = new SAXBuilder();
    InputStream stream = null;
    try {
      stream = new FileInputStream(path);
      Document doc = sb.build(stream);
      Element root = doc.getRootElement();
      Element elem;

      if ((elem = root.getChild("replies")) != null) {
        if ((elem = elem.getChild("destination-address")) != null) {
          address = elem.getTextTrim();
        }
        else{
          throw new Exception("Section 'destination-address' doesn't exist in "+path);
        }
      } else {
        throw new Exception("Section 'replies' doesn't exist in "+path);
      }
      if ((elem = root.getChild("general")) != null) {
        Element subElem;
        if ((subElem = elem.getChild("date-begin")) != null) {
          dateBegin = dateFormat.parse(subElem.getTextTrim());
        } else {
          throw new Exception("Section 'date-begin' doesn't exist in "+path);
        }
        if ((subElem = elem.getChild("date-end")) != null) {
          dateEnd = dateFormat.parse(subElem.getTextTrim());
        } else {
          throw new Exception("Section 'date-end' doesn't exist in "+path);
        }
      } else {
        throw new Exception("Section 'general' doesn't exist in "+path);
      }
    } catch (Exception e) {
      logger.error("Parsing exception", e);
      e.printStackTrace();
    } finally {
      if (stream != null) {
        try {
          stream.close();
        } catch (IOException e) {
          logger.error("Error close stream", e);
        }
      }
    }
    return new QuizXmlData(address,dateBegin,dateEnd);
  }

}
