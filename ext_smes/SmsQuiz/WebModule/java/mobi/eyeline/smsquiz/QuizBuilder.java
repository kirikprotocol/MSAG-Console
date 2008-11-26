package mobi.eyeline.smsquiz;

import org.jdom.input.SAXBuilder;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.output.XMLOutputter;
import org.jdom.output.Format;

import java.util.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.io.*;

import mobi.eyeline.smsquiz.quizes.view.QuizData;
import mobi.eyeline.smsquiz.quizes.AnswerCategory;

/**
 * author: alkhal
 * Date: 07.11.2008
 */
public class QuizBuilder {

  private static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(QuizBuilder.class);


  private static SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm");



  public static QuizData parseAll(String filepath) throws QuizParsingException {

    SAXBuilder sb = new SAXBuilder();
    InputStream stream = null;
    QuizData data = new QuizData();
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

  private static void parseGeneral(Element generalElem, QuizData data) throws QuizParsingException {
    Element elem;
    String dateBegin = null;
    String dateEnd = null;
    String question = null;
    String abFileName = null;
    String name = null;
    if ((elem = generalElem.getChild("name")) != null) {
      name = elem.getTextTrim();
    } else {
      errorNotFound("name");
    }
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
    data.setName(name);
    try {
      data.setDateBegin(dateFormat.parse(dateBegin));
      data.setDateEnd(dateFormat.parse(dateEnd));
    } catch (ParseException e) {
      logger.error(e,e);
      throw new QuizParsingException(e.toString(),e);
    }
    data.setAbFile(abFileName);

    data.setQuestion(question);

  }

  private static void parseDistribution(Element distrlElem, QuizData data) throws QuizParsingException {
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
    try{
      data.setDistrDateEnd(dateFormat.parse(distrDateEnd));
    }catch(Exception e) {
      logger.error(e,e);
      throw new QuizParsingException(e);
    }
  }

  private static void parseReplies(Element repliesElem, QuizData data) throws QuizParsingException {
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


  public static void saveQuiz(QuizData data, String filePath) {
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

  private static Element buildGeneral(QuizData data) {
    Element general = new Element("general");
    Element element = new Element("date-begin");
    element.setText(data.getDateBeginStr().trim());
    general.addContent(element);

    element = new Element("name");
    element.setText(data.getName().trim());
    general.addContent(element);

    element = new Element("date-end");
    element.setText(data.getDateEndStr().trim());
    general.addContent(element);

    element = new Element("question");
    element.setText(data.getQuestion().trim());
    general.addContent(element);

    element = new Element("abonents-file");
    element.setText(data.getAbFile().trim());
    general.addContent(element);

    return general;

  }

  private static Element buildDistribution(QuizData data) {
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
    element.setText(data.getDistrDateEndStr().trim());
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

  private static Element buildReplies(QuizData data) {
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
