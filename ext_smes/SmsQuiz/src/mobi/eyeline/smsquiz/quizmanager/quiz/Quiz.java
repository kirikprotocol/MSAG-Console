package mobi.eyeline.smsquiz.quizmanager.quiz;

import com.eyeline.jstore.JStore;
import mobi.eyeline.smsquiz.distribution.DistributionException;
import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.distribution.StatsDelivery;
import mobi.eyeline.smsquiz.quizmanager.QuizException;
import mobi.eyeline.smsquiz.quizmanager.Result;
import mobi.eyeline.smsquiz.replystats.Reply;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyDataSourceException;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyStatsDataSource;
import mobi.eyeline.smsquiz.storage.ResultSet;
import mobi.eyeline.smsquiz.storage.StorageException;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * author: alkhal
 */

public class Quiz {
  private static final Logger logger = Logger.getLogger(Quiz.class);

  private String destAddress; //At this number come reply
  private String sourceAddress;
  private JStore jstore;
  private String question;
  private String fileName;
  private Date dateBegin;
  private Date dateEnd;

  private int maxRepeat;
  private String defaultCategory;
  private String dirResult;
  private String quizName;

  private List<ReplyPattern> replyPatterns;

  private ReplyStatsDataSource replyStatsDataSource;
  private final DistributionManager distributionManager;

  private int answerHandled = -5;

  private Status status;
  private String dirWork;

  public Quiz(final String statusDir, final File file,
              final ReplyStatsDataSource replyStatsDataSource,
              final DistributionManager distributionManager, final String dirResult, final String dirWork) throws QuizException {
    this.dirResult = dirResult;
    this.replyStatsDataSource = replyStatsDataSource;
    this.distributionManager = distributionManager;
    this.dirWork = dirWork;
    jstore = new JStore(-1);
    if (file == null) {
      logger.error("Some arguments are null");
      throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    fileName = file.getAbsolutePath();
    quizName = file.getName().substring(0, file.getName().lastIndexOf("."));
    jstore.init(dirWork+File.separator+file.getName()+ ".bin", 60000, 10);
    status = new Status(statusDir + File.separator + quizName + ".status");
    replyPatterns = new ArrayList<ReplyPattern>();
  }

  public String getStatusFileName() {
    return status.getStatusFileName(); 
  }

  public Result handleSms(String oa, String text) throws QuizException {
    Result result;
    long oaNumber = Long.parseLong(oa.substring(oa.lastIndexOf("+") + 1, oa.length()));

    int count = jstore.get(oaNumber);
    if((count==answerHandled)) {
      if (logger.isInfoEnabled()) {
        logger.info("DON'T HANDLE: already handled for abonent: " + oa);
      }
      return null;
    }
    ReplyPattern replyPattern = getReplyPattern(text);
    if (replyPattern != null) {
      if (count == maxRepeat) {
        if (logger.isInfoEnabled()) {
          logger.info("DON'T HANDLE: Max repeat for abonent: " + oa);
        }
        return null;
      }
      jstore.put(oaNumber, answerHandled);
      result = new Result(replyPattern.getAnswer(), Result.ReplyRull.OK, sourceAddress);

    } else {
      if (maxRepeat > 0) {
        if (count != -1) {
          if (count >= maxRepeat) {
            jstore.put(oaNumber, answerHandled);
            return null;
          } else {
            count++;
            jstore.put(oaNumber, count);
            result = new Result(question, Result.ReplyRull.REPEAT, sourceAddress);
          }
        } else {
          jstore.put(oaNumber, 1);
          result = new Result(question, Result.ReplyRull.REPEAT, sourceAddress);
        }
      } else {
        jstore.put(oaNumber, 0);
        if (logger.isInfoEnabled()) {
          logger.info("DON'T HANDLE: Max repeat for abonent: " + oa);
          logger.info("Max repeat=0");
        }
        result = null;
      }
    }

    try {

      replyStatsDataSource.add(new Reply(new Date(), oa, destAddress, text));

      if (logger.isInfoEnabled()) {
        logger.info("Sms stored: " + oa + " " + destAddress + " " + text);
      }
    } catch (ReplyDataSourceException e) {
      logger.error("Can't add reply", e);
      throw new QuizException("Can't add reply", e);
    }
    if((result!=null)&&(result.getReplyRull().equals(Result.ReplyRull.REPEAT))) {
      try {
        distributionManager.resend(oa, status.getId());
      } catch (DistributionException e) {
        e.printStackTrace();
        logger.error("Can't resend the message", e);
        throw new QuizException("Can't resend the message", e);
      }
    }
    return result;

  }

  public void shutdown() {
    jstore.shutdown();
  }

  public void exportStats() throws QuizException {
    if (logger.isInfoEnabled()) {
      logger.info("Export statistics begining for: " + fileName);
    }
    SimpleDateFormat dateFormat = new SimpleDateFormat("ddMMyy_HHmmss");
    String fileName = dirResult + File.separator + quizName + "." + dateFormat.format(dateBegin) + "-" + dateFormat.format(dateEnd) + ".res";
    dateFormat = new SimpleDateFormat("dd.MM.yy HH:mm:ss");

    File file = new File(fileName);
    File parentFile = file.getParentFile();
    if ((parentFile != null) && (!parentFile.exists())) {
      parentFile.mkdirs();
    }
    PrintWriter printWriter = null;
    String encoding = System.getProperty("file.encoding");
    try {
      printWriter = new PrintWriter(file,encoding);
      ResultSet resultSet = distributionManager.getStatistics(this.getId(), dateBegin, dateEnd);
      String comma = ",";
      while (resultSet.next()) {
        Reply reply;
        StatsDelivery delivery = (StatsDelivery) resultSet.get();
        String oa = delivery.getMsisdn();
        Date dateDelivery = delivery.getDate();
        if (logger.isInfoEnabled()) {
          logger.info("Analysis delivery: " + delivery);
        }
        if ((reply = replyStatsDataSource.getLastReply(oa, destAddress, dateBegin, dateEnd)) != null) {
          String text = reply.getText();
          ReplyPattern replyPattern = getReplyPattern(text);
          String category;
          if (replyPattern != null) {
            category = replyPattern.getCategory();
          } else {
            if (defaultCategory != null) {
              category = defaultCategory;
            } else {
              continue;
            }
          }
         /* String oaEnc = new String(oa.getBytes(),encoding);
          String deliveryDate  = new String(dateFormat.format(dateDelivery).getBytes(),encoding);
          String replyDate  = new String(dateFormat.format(reply.getDate()).getBytes(),encoding);
          String categoryEnc  = new String(category.getBytes(),encoding);
          String replyText = reply.getText().replace(System.getProperty("line.separator"), "\\n");
          replyText = new String(replyText.getBytes(),encoding);
          printWriter.print(oaEnc);
          printWriter.print(comma);
          printWriter.print(deliveryDate);
          printWriter.print(comma);
          printWriter.print(replyDate);
          printWriter.print(comma);
          printWriter.print(categoryEnc);
          printWriter.print(comma);
          printWriter.println(replyText);    */
          printWriter.print(oa);
          printWriter.print(comma);
          printWriter.print(dateFormat.format(dateDelivery));
          printWriter.print(comma);
          printWriter.print(dateFormat.format(reply.getDate()));
          printWriter.print(comma);
          printWriter.print(category);
          printWriter.print(comma);
          printWriter.println(reply.getText().replace(System.getProperty("line.separator"), "\\n"));
          if (logger.isInfoEnabled()) {
            logger.info("Last reply for oa=" + oa + " is " + reply + " stored");
          }
        } else {
          if (logger.isInfoEnabled()) {
            logger.info("Last reply for oa=" + oa + " is empty");
          }
        }
      }
      printWriter.flush();
    } catch (DistributionException e) {
      logger.error("Can't get statisctics", e);
      throw new QuizException("Can't get statisctics", e);
    } catch (ReplyDataSourceException e) {
      logger.error("Can't get replies", e);
      throw new QuizException("Can't get replies", e);
    } catch (StorageException e) {
      logger.error("Can't read ResultSet", e);
      throw new QuizException("Can't read ResultSet", e);
    } catch (IOException e) {
      logger.error("Error during writing file", e);
      throw new QuizException("Error during writing file", e);
    } finally {
      if (printWriter != null) {
        printWriter.close();
      }
    }
    logger.info("Export statistics finished");
  }

  public Date getDateBegin() {
    return dateBegin;
  }

  public void setDateBegin(Date dateBegin) {
    this.dateBegin = dateBegin;
  }

  public Date getDateEnd() {
    return dateEnd;
  }

  public void setDateEnd(Date dateEnd) {
    this.dateEnd = dateEnd;
  }

  public String getQuestion() {
    return question;
  }

  public void setQuestion(String question) {
    this.question = question;
  }

  public String getFileName() {
    return fileName;
  }

  public void setFileName(String fileName) {
    this.fileName = fileName;
  }

  public String getDestAddress() {
    return destAddress;
  }

  public void setDestAddress(String destAddress) {
    this.destAddress = destAddress;
  }

  public String getId() {
    return status.getId();
  }

  public void setId(String id) throws QuizException {
    status.setId(id);
  }

  public void addReplyPattern(ReplyPattern replyPattern) {
    if (replyPattern != null) {
      replyPatterns.add(replyPattern);
    }
  }

  public void clearPatterns() {
    replyPatterns.clear();
  }

  public void setDefaultCategory(String category) {
    if (category != null) {
      defaultCategory = category;
    }
  }

  public void setMaxRepeat(int maxRepeat) {
    this.maxRepeat = maxRepeat;
  }

  private ReplyPattern getReplyPattern(String text) {
    if (text != null) {
      for (ReplyPattern rP : replyPatterns) {
        if (rP.matches(text)) {
          return rP;
        }
      }
    }
    return null;

  }

  public String toString() {
    String sep = System.getProperty("line.separator");
    StringBuilder strBuilder = new StringBuilder();
    strBuilder.append(sep);
    strBuilder.append("fileName: ").append(fileName).append(sep);
    strBuilder.append("address: ").append(destAddress).append(sep);
    strBuilder.append("question: ").append(question).append(sep);
    strBuilder.append("dateBegin: ").append(dateBegin).append(sep);
    strBuilder.append("dateEnd: ").append(dateEnd).append(sep);
    strBuilder.append("id: ").append(this.getId()).append(sep);
    return strBuilder.substring(0);
  }

  public String getSourceAddress() {
    return sourceAddress;
  }

  public void setSourceAddress(String sourceAddress) {
    this.sourceAddress = sourceAddress;
  }

  public boolean isActive() {
    Date now = new Date();
    return now.after(dateBegin)&&now.before(dateEnd);
  }

  public String getQuizName() {
    return quizName;
  }

}