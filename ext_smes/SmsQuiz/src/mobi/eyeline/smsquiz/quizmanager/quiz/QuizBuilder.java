package mobi.eyeline.smsquiz.quizmanager.quiz;

import mobi.eyeline.smsquiz.distribution.Distribution;
import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import mobi.eyeline.smsquiz.quizmanager.QuizException;
import org.jdom.input.SAXBuilder;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.io.FileInputStream;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.util.*;


/**
 * author: alkhal
 */
public class QuizBuilder {
    private static Logger logger = Logger.getLogger(QuizBuilder.class);
    private SimpleDateFormat dateFormat;
    private String timeSeparator;


    public QuizBuilder (String dPattern, String tSeparotor) {
        timeSeparator = tSeparotor;
        dateFormat = new SimpleDateFormat(dPattern);
    }

    public void buildQuiz(final String filepath, Distribution distribution, Quiz quiz) throws QuizException {
        if(quiz == null) {
            logger.error("Argument 'quiz' is null");
            throw new QuizException("Argument 'quiz' is null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
        }

        SAXBuilder sb = new SAXBuilder();
        try {
            Document doc = sb.build(new FileInputStream(filepath));
            Element root = doc.getRootElement();
            Element elem = null;
            if((elem = root.getChild("general"))!=null) {
                parseGeneral(elem, distribution, quiz);
            }
            else {
                errorNotFound("general");
            }
            if((elem = root.getChild("distribution"))!=null) {
                parseDistribution(elem, distribution, quiz);
            }
            else {
                errorNotFound("distribution");
            }
            if((elem = root.getChild("replies"))!=null) {
                parseReplies(elem, quiz);
            }
            else {
                errorNotFound("replies");
            }
        } catch (JDOMException e) {
            logger.error("Parsing exception", e);
            throw new QuizException("Parsing exception", e);
        } catch (IOException e) {
            logger.error("Parsing exception", e);
            throw new QuizException("Parsing exception", e);
        }
    }
    private void parseGeneral(Element generalElem, Distribution distribution, Quiz quiz) throws QuizException{
        Element elem = null;
        Date dateBegin = null;
        Date dateEnd = null;
        String question = null;
        String abFileName = null;
        try {
            if((elem = generalElem.getChild("date-begin"))!=null) {
                dateBegin = dateFormat.parse(elem.getTextTrim());
            } else {
                errorNotFound("date-begin");
            }
            if((elem = generalElem.getChild("date-end"))!=null) {
                dateEnd = dateFormat.parse(elem.getTextTrim());
            } else {
                errorNotFound("date-end");
            }
            if((elem = generalElem.getChild("question"))!=null) {
                question = elem.getTextTrim();
            } else {
                errorNotFound("question");
            }
            if((elem = generalElem.getChild("abonents-file"))!=null) {
                abFileName = elem.getTextTrim();
            } else {
                errorNotFound("abonents-file");
            }
        } catch (ParseException e) {
            logger.error("Parsing exception", e);
            throw new QuizException("Parsing exception", e);
        }
        if(distribution!=null) {
            distribution.setDateBegin(dateBegin);
            distribution.setDateEnd(dateEnd);
            distribution.setFilePath(abFileName);
        }

        quiz.setDateBegin(dateBegin);
        quiz.setDateEnd(dateEnd);
        quiz.setQuestion(question);

        /*
        <date-begin>10.10.2008 09:00</date-begin>
        <date-end>15.20.2008 22:00</date-end>
        <question>Question</question>
        <abonents-file>opros1/abonents.csv</abonents-file>*/
    }

    @SuppressWarnings({"unchecked"})
    private void parseDistribution(Element distrlElem, Distribution distribution, Quiz quiz) throws QuizException {
        Element elem = null;
        String sourceaddress = null;
        Calendar timeBegin = Calendar.getInstance();
        Calendar timeEnd = Calendar.getInstance();
        String txmode = null;

        if((elem = distrlElem.getChild("source-address"))!=null) {
            sourceaddress = elem.getTextTrim();
        } else {
                errorNotFound("source-address");
        }

        if((elem = distrlElem.getChild("time-begin"))!=null) {
            String time = elem.getTextTrim();
            makeTime(timeBegin,time);
        } else {
            errorNotFound("time-begin");
        }

        if((elem = distrlElem.getChild("time-end"))!=null) {
            String time = elem.getTextTrim();
            makeTime(timeEnd,time);
        } else {
            errorNotFound("time-end");
        }
        if(distribution!=null) {
            if((elem = distrlElem.getChild("days"))!=null) {
                List<Element> list = null;
                if((list = elem.getChildren("day"))!=null) {
                    for(Element el: list) {
                        Distribution.WeekDays day;
                        if((day = Distribution.WeekDays.valueOf(el.getTextTrim().toUpperCase()))!=null) {
                            distribution.addDay(day);
                        }
                        else {
                            logger.error("Error during parsing week days, unsupported format of day: "+el.getTextTrim());
                            throw new QuizException("Error during parsing week days, unsupported format of day: "+el.getTextTrim());
                        }
                    }
                }
                else {
                    errorNotFound("day");
                }
            } else {
                errorNotFound("days");
            }
        }
        if((elem = distrlElem.getChild("time-end"))!=null) {
            String time = elem.getTextTrim();
            makeTime(timeEnd,time);
        } else {
            errorNotFound("time-end");
        }

        if((elem = distrlElem.getChild("txmode"))!=null) {
            txmode = elem.getTextTrim();
        } else {
            errorNotFound("txmode");
        }

        if(distribution!=null) {
            distribution.setTimeBegin(timeBegin);
            distribution.setTimeEnd(timeEnd);
            distribution.setTxmode(txmode);
            distribution.setSourceAddress(sourceaddress);
        }
        quiz.setSourceAddress(sourceaddress);


       /*         <source-address>148</source-address>
        <time-begin>12:00</time-begin>
        <time-end>20:00</time-end>
        <days>
            <day>Mon</day>
            <day>Sun</day>
        </days>
        <txmode>transaction</txmode>  */
    }
    @SuppressWarnings({"unchecked"})
    private void parseReplies(Element repliesElem, Quiz quiz) throws QuizException{
        ReplyPattern replyPattern=null;
        Element elem = null;

        if((elem = repliesElem.getChild("destination-address"))!=null) {
            quiz.setDestAddress(elem.getTextTrim());
        } else {
            errorNotFound("destination-address");
        }
        if((elem = repliesElem.getChild("max-repeat"))!=null) {
            try{
                quiz.setMaxRepeat(Integer.parseInt(elem.getTextTrim()));
            } catch (NumberFormatException e) {
                logger.error("Unsupported format for integer: "+elem.getTextTrim());
                throw new QuizException("Unsupported format for integer: "+elem.getTextTrim());
            }
        }
        else {
            quiz.setMaxRepeat(0);
        }
        if((elem = repliesElem.getChild("default"))!=null) {
            quiz.setDefaultCategory(elem.getTextTrim());
        }
        List<Element> list = null;
        if((list = repliesElem.getChildren("reply"))!=null) {
            quiz.clearPatterns();
            for(Element el:list) {
                Element subEl=null;
                String category = null;
                String answer = null;
                String pattern = null;
                if((subEl = el.getChild("category"))!=null) {
                    category = subEl.getTextTrim();
                } else {
                    errorNotFound("category");
                }
                if((subEl = el.getChild("pattern"))!=null) {
                    pattern = subEl.getTextTrim();
                } else {
                    errorNotFound("pattern");
                }
                if((subEl = el.getChild("answer"))!=null) {
                    answer = subEl.getTextTrim();
                } else {
                    errorNotFound("answer");
                }
                quiz.addReplyPattern(new ReplyPattern(pattern, category, answer));
            }
        } else {
            errorNotFound("reply");
        }

   /* <replies>
        <destination-address>148</destination-address>
        <max-repeat>3</max-repeat>

        <reply>
            <category>Да</category>
            <pattern>(yes|y|1|да|д)</pattern>
            <answer>Thanks</answer>
        </reply>

        <reply>
            <category>Нет</category>
            <pattern>(no|n|0|нет|н)</pattern>
            <answer>Thanks</answer>
        </reply>

        <default>Да</default>

    </replies>   */

    }
    private void errorNotFound(String element) throws QuizException{
        logger.error("Parsing exception, element not found: "+ element);
        throw new QuizException("Parsing exception, element not found"+ element);
    }
    private void makeTime(Calendar cal, String time) throws QuizException{
        StringTokenizer tokenizer = new StringTokenizer(time,timeSeparator);
        try {
            cal.set(Calendar.HOUR_OF_DAY, Integer.parseInt(tokenizer.nextToken()));
            cal.set(Calendar.MINUTE, Integer.parseInt(tokenizer.nextToken()));
        } catch (NumberFormatException exc) {
            logger.error("Unsupported time format", exc);
            throw new QuizException("Unsupported time format", exc);
        }
    }

}
