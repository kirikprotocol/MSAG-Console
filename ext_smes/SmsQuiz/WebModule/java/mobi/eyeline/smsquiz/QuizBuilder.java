package mobi.eyeline.smsquiz;

import org.jdom.input.SAXBuilder;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.output.XMLOutputter;
import org.jdom.output.Format;

import java.util.*;
import java.text.SimpleDateFormat;
import java.io.*;

import mobi.eyeline.smsquiz.quizes.view.QuizFullData;
import mobi.eyeline.smsquiz.quizes.AnswerCategory;

/**
 * author: alkhal
 * Date: 07.11.2008
 */
public class QuizBuilder {

  private static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(QuizBuilder.class);

  private static String datePattern = "dd.MM.yyyy HH:mm";


  public static QuizShortData parseQuiz(String path) throws QuizParsingException {
    String abFile = null;
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
        } else {
          throw new Exception("Section 'destination-address' doesn't exist in " + path);
        }
      } else {
        throw new Exception("Section 'replies' doesn't exist in " + path);
      }
      if ((elem = root.getChild("general")) != null) {
        Element subElem;
        if ((subElem = elem.getChild("date-begin")) != null) {
          dateBegin = dateFormat.parse(subElem.getTextTrim());
        } else {
          throw new Exception("Section 'date-begin' doesn't exist in " + path);
        }
        if ((subElem = elem.getChild("date-end")) != null) {
          dateEnd = dateFormat.parse(subElem.getTextTrim());
        } else {
          throw new Exception("Section 'date-end' doesn't exist in " + path);
        }
        if ((subElem = elem.getChild("abonents-file")) != null) {
         abFile = subElem.getTextTrim();
        } else {
          throw new Exception("Section 'abonents-file' doesn't exist in " + path);
        }
      } else {
        throw new Exception("Section 'general' doesn't exist in " + path);
      }
    } catch (Exception e) {
      logger.error("Parsing exception", e);
      e.printStackTrace();
      throw new QuizParsingException(e);
    } finally {
      if (stream != null) {
        try {
          stream.close();
        } catch (IOException e) {
          logger.error("Error close stream", e);
        }
      }
    }
    return new QuizShortData(address, dateBegin, dateEnd, abFile);
  }

  public static QuizFullData parseAll(String filepath) throws QuizParsingException {
    File file = new File(filepath);
    String quizName = file.getName().substring(0, file.getName().indexOf("."));

    SAXBuilder sb = new SAXBuilder();
    InputStream stream = null;
    QuizFullData data = new QuizFullData();
    data.setQuiz(quizName);
    try {
      stream = new FileInputStream(filepath);
      Document doc = sb.build(stream);
      Element root = doc.getRootElement();
      Element elem;
      if ((elem = root.getChild("general")) != null) {
        parseGeneral(elem, data);
      } else {
        errorNotFound("general");
      }
      if ((elem = root.getChild("distribution")) != null) {
        parseDistribution(elem, data);
      } else {
        errorNotFound("distribution");
      }
      if ((elem = root.getChild("replies")) != null) {
        parseReplies(elem, data);
      } else {
        errorNotFound("replies");
      }
    } catch (Exception e) {
      logger.error("Parsing exception", e);
      throw new QuizParsingException("Parsing exception", e);
    } finally {
      if (stream != null) {
        try {
          stream.close();
        } catch (IOException e) {
          logger.error("Error close stream", e);
        }
      }
    }
    return data;
  }

  private static void parseGeneral(Element generalElem, QuizFullData data) throws QuizParsingException {
    Element elem;
    String dateBegin = null;
    String dateEnd = null;
    String question = null;
    String abFileName = null;
    if ((elem = generalElem.getChild("date-begin")) != null) {
      dateBegin = elem.getTextTrim();
    } else {
      errorNotFound("date-begin");
    }
    if ((elem = generalElem.getChild("date-end")) != null) {
      dateEnd = elem.getTextTrim();
    } else {
      errorNotFound("date-end");
    }
    if ((elem = generalElem.getChild("question")) != null) {
      question = elem.getTextTrim();
    } else {
      errorNotFound("question");
    }
    if ((elem = generalElem.getChild("abonents-file")) != null) {
      abFileName = elem.getTextTrim();
    } else {
      errorNotFound("abonents-file");
    }
    data.setDateBegin(dateBegin);
    data.setDateEnd(dateEnd);
    data.setAbFile(abFileName);

    data.setQuestion(question);

  }

  private static void parseDistribution(Element distrlElem, QuizFullData data) throws QuizParsingException {
    Element elem;
    String sourceaddress = null;
    String timeBegin = null;
    String timeEnd = null;
    String txmode = null;
    String distrDateEnd = null;

    if ((elem = distrlElem.getChild("source-address")) != null) {
      sourceaddress = elem.getTextTrim();
    } else {
      errorNotFound("source-address");
    }

    if ((elem = distrlElem.getChild("time-begin")) != null) {
      timeBegin = elem.getTextTrim();
    } else {
      errorNotFound("time-begin");
    }

    if ((elem = distrlElem.getChild("time-end")) != null) {
      timeEnd = elem.getTextTrim();
    } else {
      errorNotFound("time-end");
    }

    if ((elem = distrlElem.getChild("date-end")) != null) {
      distrDateEnd = elem.getTextTrim();
    } else {
      errorNotFound("date-end");
    }

    if ((elem = distrlElem.getChild("days")) != null) {
      List list;
      if ((list = elem.getChildren("day")) != null) {
        Iterator iter = list.iterator();
        while (iter.hasNext()) {
          data.addActiveDay(((Element) iter.next()).getTextTrim());
        }
      } else {
        errorNotFound("day");
      }
    } else {
      errorNotFound("days");
    }

    if ((elem = distrlElem.getChild("txmode")) != null) {
      txmode = elem.getTextTrim();
    } else {
      errorNotFound("txmode");
    }

    data.setTimeBegin(timeBegin);
    data.setTimeEnd(timeEnd);
    data.setTxmode(txmode);
    data.setSourceAddress(sourceaddress);
    data.setDistrDateEnd(distrDateEnd);
  }

  private static void parseReplies(Element repliesElem, QuizFullData data) throws QuizParsingException {
    Element elem;

    if ((elem = repliesElem.getChild("destination-address")) != null) {
      data.setDestAddress(elem.getTextTrim());
    } else {
      errorNotFound("destination-address");
    }
    if ((elem = repliesElem.getChild("max-repeat")) != null) {
      data.setMaxRepeat(elem.getTextTrim());
    } else {
      data.setMaxRepeat("0");
    }
    if ((elem = repliesElem.getChild("default")) != null) {
      data.setDefaultCategory(elem.getTextTrim());
    }
    List list;
    if ((list = repliesElem.getChildren("reply")) != null) {
      Iterator iter = list.iterator();
      while (iter.hasNext()) {
        Element el = (Element) iter.next();
        Element subEl;
        String category = null;
        String answer = null;
        String pattern = null;
        if ((subEl = el.getChild("category")) != null) {
          category = subEl.getTextTrim();
        } else {
          errorNotFound("category");
        }
        if ((subEl = el.getChild("pattern")) != null) {
          pattern = subEl.getTextTrim();
        } else {
          errorNotFound("pattern");
        }
        if ((subEl = el.getChild("answer")) != null) {
          answer = subEl.getTextTrim();
        } else {
          errorNotFound("answer");
        }
        data.addCategory(new AnswerCategory(pattern, category, answer));
      }
    } else {
      errorNotFound("reply");
    }

  }

  private static void errorNotFound(String element) throws QuizParsingException {
    logger.error("Parsing exception, element not found: " + element);
    throw new QuizParsingException("Parsing exception, element not found" + element);
  }


  public static void saveQuiz(QuizFullData data, String filePath) {
    Format format = Format.getPrettyFormat();
    format.setEncoding(System.getProperty("file.encoding"));
    XMLOutputter outputter = new XMLOutputter(format);


    OutputStream outputStream = null;
    try {
      outputStream = new FileOutputStream(filePath);
      Document doc = new Document();
      Element root = new Element("opros");
      doc.setRootElement(root);
      root.addContent(buildGeneral(data));
      root.addContent(buildDistribution(data));
      root.addContent(buildReplies(data));
      outputter.output(doc, outputStream);

    } catch (IOException e) {
      logger.error(e);
      e.printStackTrace();
    } finally {
      if (outputStream != null) {
        try {
          outputStream.close();
        } catch (IOException e) {
          e.printStackTrace();
        }
      }
    }
  }

  private static Element buildGeneral(QuizFullData data) {
    Element general = new Element("general");
    Element element = new Element("date-begin");
    element.setText(data.getDateBegin().trim());
    general.addContent(element);

    element = new Element("date-end");
    element.setText(data.getDateEnd().trim());
    general.addContent(element);

    element = new Element("question");
    element.setText(data.getQuestion().trim());
    general.addContent(element);

    element = new Element("abonents-file");
    element.setText(data.getAbFile().trim());
    general.addContent(element);

    return general;

  }

  private static Element buildDistribution(QuizFullData data) {
    Element distr = new Element("distribution");

    Element element = new Element("source-address");
    element.setText(data.getSourceAddress().trim());
    distr.addContent(element);

    element = new Element("time-begin");
    element.setText(data.getTimeBegin().trim());
    distr.addContent(element);

    element = new Element("time-end");
    element.setText(data.getTimeEnd().trim());
    distr.addContent(element);

    element = new Element("date-end");
    element.setText(data.getDistrDateEnd().trim());
    distr.addContent(element);

    element = new Element("source-address");
    element.setText(data.getSourceAddress().trim());
    distr.addContent(element);


    element = new Element("days");
    Element subEl;

    Iterator iter = data.getActiveDays().iterator();
    while (iter.hasNext()) {
      String day = (String) iter.next();
      subEl = new Element("day");
      subEl.setText(day.trim());
      element.addContent(subEl);
    }
    distr.addContent(element);

    element = new Element("txmode");
    element.setText((Boolean.valueOf(data.getTxmode().trim())).toString());
    distr.addContent(element);

    return distr;
  }

  private static Element buildReplies(QuizFullData data) {
    Element replies = new Element("replies");

    Element element = new Element("destination-address");
    element.setText(data.getDestAddress().trim());
    replies.addContent(element);

    String maxRepeat = data.getMaxRepeat();
    if ((maxRepeat != null) && (!maxRepeat.trim().equals(""))) {
      maxRepeat = maxRepeat.trim();
      element = new Element("max-repeat");
      element.setText(maxRepeat);
      replies.addContent(element);
    }

    String def = data.getDefaultCategory();
    if ((def != null) && (!def.trim().equals(""))) {
      def = def.trim();
      element = new Element("default");
      element.setText(def);
      replies.addContent(element);
    }

    Iterator iter = data.getCategoriesIter();
    while (iter.hasNext()) {
      element = new Element("reply");
      AnswerCategory cat = (AnswerCategory) iter.next();

      Element subEl = new Element("category");
      subEl.setText(cat.getName().trim());
      element.addContent(subEl);

      subEl = new Element("pattern");
      subEl.setText(cat.getPattern().trim());
      element.addContent(subEl);

      subEl = new Element("answer");
      subEl.setText(cat.getAnswer().trim());
      element.addContent(subEl);

      replies.addContent(element);
    }
    return replies;
  }

}
