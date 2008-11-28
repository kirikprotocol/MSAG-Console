package mobi.eyeline.smsquiz.quizmanager.quiz;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;
import mobi.eyeline.smsquiz.distribution.Distribution;
import mobi.eyeline.smsquiz.quizmanager.QuizException;
import org.apache.log4j.Logger;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.StringTokenizer;


/**
 * author: alkhal
 */
public class QuizBuilder {
  private static final Logger logger = Logger.getLogger(QuizBuilder.class);
  private final SimpleDateFormat dateFormat;
  private final String timeSeparator;
  private final String dPattern;

  private AbstractDynamicMBean monitor;

  public QuizBuilder(String dPattern, String tSeparotor) {
    this.timeSeparator = tSeparotor;
    this.dPattern = dPattern;
    dateFormat = new SimpleDateFormat(dPattern);
    monitor = new QuizBuilderMBean(this);
  }

  public void buildQuiz(final String filepath, Quiz quiz) throws QuizException {
    if (quiz == null) {
      logger.error("Some argument are null");
      throw new QuizException("Some argument are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    Distribution distribution = quiz.getDistribution();
    SAXBuilder sb = new SAXBuilder();
    InputStream stream = null;
    try {
      stream = new FileInputStream(filepath);
      Document doc = sb.build(stream);
      Element root = doc.getRootElement();
      Element elem;
      if ((elem = root.getChild("general")) != null) {
        parseGeneral(elem, distribution, quiz);
      } else {
        errorNotFound("general");
      }
      if ((elem = root.getChild("distribution")) != null) {
        parseDistribution(elem, distribution, quiz);
      } else {
        errorNotFound("distribution");
      }
      if ((elem = root.getChild("replies")) != null) {
        parseReplies(elem, quiz);
      } else {
        errorNotFound("replies");
      }
    } catch (JDOMException e) {
      logger.error("Parsing exception", e);
      throw new QuizException("Parsing exception", e);
    } catch (IOException e) {
      logger.error("Parsing exception", e);
      throw new QuizException("Parsing exception", e);
    } finally {
      if (stream != null) {
        try {
          stream.close();
        } catch (IOException e) {
          logger.error("Error close stream", e);
        }
      }
    }
  }

  private void parseGeneral(Element generalElem, Distribution distribution, Quiz quiz) throws QuizException {
    Element elem;
    Date dateBegin = null;
    Date dateEnd = null;
    String question = null;
    String abFileName = null;
    String name = null;
    try {
      if ((elem = generalElem.getChild("name")) != null) {
        name = elem.getTextTrim();
      } else {
        errorNotFound("name");
      }
      if ((elem = generalElem.getChild("date-begin")) != null) {
        dateBegin = dateFormat.parse(elem.getTextTrim());
      } else {
        errorNotFound("date-begin");
      }
      if ((elem = generalElem.getChild("date-end")) != null) {
        dateEnd = dateFormat.parse(elem.getTextTrim());
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
    } catch (ParseException e) {
      logger.error("Parsing exception", e);
      throw new QuizException("Parsing exception", e);
    }
    if (distribution != null) {
      distribution.setDateBegin(dateBegin);
      distribution.setFilePath(abFileName);
      distribution.setTaskName(name + "(SmsQuiz)");
    }
    if (quiz != null) {
      quiz.setDateBegin(dateBegin);
      quiz.setDateEnd(dateEnd);
      quiz.setQuestion(question);
      quiz.setQuizName(name);
      quiz.setOrigAbFile(abFileName);
    }
  }

  @SuppressWarnings({"unchecked"})
  private void parseDistribution(Element distrlElem, Distribution distribution, Quiz quiz) throws QuizException {
    Element elem;
    String sourceaddress = null;
    Calendar timeBegin = Calendar.getInstance();
    Calendar timeEnd = Calendar.getInstance();
    String txmode = null;

    if ((elem = distrlElem.getChild("source-address")) != null) {
      sourceaddress = elem.getTextTrim();
    } else {
      errorNotFound("source-address");
    }

    if ((elem = distrlElem.getChild("time-begin")) != null) {
      String time = elem.getTextTrim();
      makeTime(timeBegin, time);
    } else {
      errorNotFound("time-begin");
    }

    if ((elem = distrlElem.getChild("time-end")) != null) {
      String time = elem.getTextTrim();
      makeTime(timeEnd, time);
    } else {
      errorNotFound("time-end");
    }
    if (distribution != null) {
      if ((elem = distrlElem.getChild("days")) != null) {
        List<Element> list;
        if ((list = elem.getChildren("day")) != null) {
          for (Element el : list) {
            Distribution.WeekDays day;
            if ((day = Distribution.WeekDays.valueOf(el.getTextTrim())) != null) {
              distribution.addDay(day);
            } else {
              logger.error("Error during parsing week days, unsupported format of day: " + el.getTextTrim());
              throw new QuizException("Error during parsing week days, unsupported format of day: " + el.getTextTrim());
            }
          }
        } else {
          errorNotFound("day");
        }
      } else {
        errorNotFound("days");
      }
      if ((elem = distrlElem.getChild("date-end")) != null) {
        try {
          distribution.setDateEnd(dateFormat.parse(elem.getTextTrim()));
        } catch (ParseException e) {
          logger.error("Parsing exception", e);
          throw new QuizException("Parsing exception", e);
        }
      } else {
        errorNotFound("date-end");
      }
    }
    if ((elem = distrlElem.getChild("time-end")) != null) {
      String time = elem.getTextTrim();
      makeTime(timeEnd, time);
    } else {
      errorNotFound("time-end");
    }

    if ((elem = distrlElem.getChild("txmode")) != null) {
      txmode = elem.getTextTrim();
    } else {
      errorNotFound("txmode");
    }

    if (distribution != null) {
      distribution.setTimeBegin(timeBegin);
      distribution.setTimeEnd(timeEnd);
      distribution.setTxmode(Boolean.valueOf(txmode));
      distribution.setSourceAddress(sourceaddress);
    }
    quiz.setSourceAddress(sourceaddress);

  }

  @SuppressWarnings({"unchecked"})
  private void parseReplies(Element repliesElem, Quiz quiz) throws QuizException {
    Element elem;

    if ((elem = repliesElem.getChild("destination-address")) != null) {
      quiz.setDestAddress(elem.getTextTrim());
    } else {
      errorNotFound("destination-address");
    }
    if ((elem = repliesElem.getChild("max-repeat")) != null) {
      try {
        quiz.setMaxRepeat(Integer.parseInt(elem.getTextTrim()));
      } catch (NumberFormatException e) {
        logger.error("Unsupported format for integer: " + elem.getTextTrim());
        throw new QuizException("Unsupported format for integer: " + elem.getTextTrim());
      }
    } else {
      quiz.setMaxRepeat(0);
    }
    if ((elem = repliesElem.getChild("default")) != null) {
      quiz.setDefaultCategory(elem.getTextTrim());
    }
    List<Element> list;
    if ((list = repliesElem.getChildren("reply")) != null) {
      quiz.clearPatterns();
      for (Element el : list) {
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
        quiz.addReplyPattern(new ReplyPattern(pattern, category, answer));
      }
    } else {
      errorNotFound("reply");
    }
  }

  public void buildModifyActive(final String filepath, Quiz quiz) throws QuizException {
    if ((filepath == null) || (quiz == null)) {
      logger.error("Some argument are null");
      throw new QuizException("Some argument are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    SAXBuilder sb = new SAXBuilder();
    InputStream stream = null;
    try {
      stream = new FileInputStream(filepath);
      Document doc = sb.build(stream);
      Element root = doc.getRootElement();
      Element repliesElem;
      if ((repliesElem = root.getChild("replies")) != null) {
        Element elem;
        if ((elem = repliesElem.getChild("max-repeat")) != null) {
          try {
            quiz.setMaxRepeat(Integer.parseInt(elem.getTextTrim()));
          } catch (NumberFormatException e) {
            logger.error("Unsupported format for integer: " + elem.getTextTrim());
            throw new QuizException("Unsupported format for integer: " + elem.getTextTrim());
          }
        } else {
          quiz.setMaxRepeat(0);
        }
        if ((elem = repliesElem.getChild("default")) != null) {
          quiz.setDefaultCategory(elem.getTextTrim());
        }
      } else {
        errorNotFound("replies");
      }
    } catch (JDOMException e) {
      logger.error("Parsing exception", e);
      throw new QuizException("Parsing exception", e);
    } catch (IOException e) {
      logger.error("Parsing exception", e);
      throw new QuizException("Parsing exception", e);
    } finally {
      if (stream != null) {
        try {
          stream.close();
        } catch (IOException e) {
          logger.error("Error close stream", e);
        }
      }
    }
  }

  @SuppressWarnings({"unchecked"})
  public void buildModifyUnactive(final String filepath, Quiz quiz) throws QuizException {
    if ((filepath == null) || (quiz == null)) {
      logger.error("Some argument are null");
      throw new QuizException("Some argument are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    SAXBuilder sb = new SAXBuilder();
    InputStream stream = null;
    try {
      stream = new FileInputStream(filepath);
      Document doc = sb.build(stream);
      Element root = doc.getRootElement();
      Element repliesElem;
      if ((repliesElem = root.getChild("replies")) != null) {
        Element elem;
        if ((elem = repliesElem.getChild("destination-address")) != null) {
          quiz.setDestAddress(elem.getTextTrim());
        } else {
          errorNotFound("destination-address");
        }
        if ((elem = repliesElem.getChild("max-repeat")) != null) {
          try {
            quiz.setMaxRepeat(Integer.parseInt(elem.getTextTrim()));
          } catch (NumberFormatException e) {
            logger.error("Unsupported format for integer: " + elem.getTextTrim());
            throw new QuizException("Unsupported format for integer: " + elem.getTextTrim());
          }
        } else {
          quiz.setMaxRepeat(0);
        }
        if ((elem = repliesElem.getChild("default")) != null) {
          quiz.setDefaultCategory(elem.getTextTrim());
        }
        List<Element> list;
        if ((list = repliesElem.getChildren("reply")) != null) {
          quiz.clearPatterns();
          for (Element el : list) {
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
            quiz.addReplyPattern(new ReplyPattern(pattern, category, answer));
          }
        } else {
          errorNotFound("reply");
        }
      } else {
        errorNotFound("replies");
      }
    }
    catch (JDOMException e) {
      logger.error("Parsing exception", e);
      throw new QuizException("Parsing exception", e);
    } catch (IOException e) {
      logger.error("Parsing exception", e);
      throw new QuizException("Parsing exception", e);
    } finally {
      if (stream != null) {
        try {
          stream.close();
        } catch (IOException e) {
          logger.error("Error close stream", e);
        }
      }
    }
  }

  private void errorNotFound(String element) throws QuizException {
    logger.error("Parsing exception, element not found: " + element);
    throw new QuizException("Parsing exception, element not found" + element);
  }

  private void makeTime(Calendar cal, String time) throws QuizException {
    StringTokenizer tokenizer = new StringTokenizer(time, timeSeparator);
    try {
      cal.set(Calendar.HOUR_OF_DAY, Integer.parseInt(tokenizer.nextToken()));
      cal.set(Calendar.MINUTE, Integer.parseInt(tokenizer.nextToken()));
    } catch (NumberFormatException exc) {
      logger.error("Unsupported time format", exc);
      throw new QuizException("Unsupported time format", exc);
    }
  }

  String getDatePattern() {
    return dPattern;
  }

  String getTimeSeparator() {
    return timeSeparator;
  }

  public AbstractDynamicMBean getMonitor() {
    return monitor;
  }
}
