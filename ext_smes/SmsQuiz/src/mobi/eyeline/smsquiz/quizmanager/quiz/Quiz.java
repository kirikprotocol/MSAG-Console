package mobi.eyeline.smsquiz.quizmanager.quiz;

import com.eyeline.jstore.JStore;
import mobi.eyeline.smsquiz.distribution.Distribution;
import mobi.eyeline.smsquiz.distribution.DistributionException;
import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.distribution.StatsDelivery;
import mobi.eyeline.smsquiz.quizmanager.QuizException;
import mobi.eyeline.smsquiz.quizmanager.Result;
import mobi.eyeline.smsquiz.quizmanager.QuizCollector;
import mobi.eyeline.smsquiz.replystats.Reply;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyDataSourceException;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyStatsDataSource;
import mobi.eyeline.smsquiz.storage.ResultSet;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * author: alkhal
 */

@SuppressWarnings({"ResultOfMethodCallIgnored"})
public class Quiz {

  private static final Logger logger = Logger.getLogger(Quiz.class);

  public static enum Status {
    NEW, GENERATION, AWAIT, ACTIVE, EXPORTING, FINISHED, FINISHED_ERROR
  }

  private JStore jstore;
  private String fileName;

  private QuizData quizData = null;

  private long lastDistrStatusCheck = System.currentTimeMillis();

  private String dirResult;
  private String archiveDir;
  private String quizDir;
  private String workDir;
  private String quizId;

  private final ReplyStatsDataSource replyStatsDataSource;
  private final DistributionManager distributionManager;
  private final QuizCollector quizCollector;

  private int answerHandled = -5;

  private StatusFile statusFile;

  private boolean exported = false;

  private final Lock exportLock = new ReentrantLock();

  private SimpleDateFormat dateFormat = new SimpleDateFormat("ddMMyy_HHmmss");


  public Quiz(final File file, final ReplyStatsDataSource replyStatsDataSource, final QuizCollector quizCollector,
              final DistributionManager distributionManager, final String dirResult,
              final String dirWork, final String archiveDir, final String quizDir) throws QuizException {
    this.dirResult = dirResult;
    this.archiveDir = archiveDir;
    this.quizDir = quizDir;
    this.workDir = dirWork;
    this.replyStatsDataSource = replyStatsDataSource;
    this.distributionManager = distributionManager;
    this.quizCollector = quizCollector;
    jstore = new JStore(-1);
    if (file == null) {
      logger.error("Some arguments are null");
      throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    fileName = file.getAbsolutePath();
    quizId = file.getName().substring(0, file.getName().lastIndexOf("."));
    jstore.init(dirWork + File.separator + file.getName() + ".bin", 60000, 10);
    statusFile = new StatusFile(dirWork + File.separator + quizId + ".status");
  }

  public String getStatusFileName() {
    return statusFile.getStatusFileName();
  }

  public Result handleSms(String oa, String text) throws QuizException {
    Result result;
    long oaNumber = Long.parseLong(oa.substring(oa.lastIndexOf("+") + 1, oa.length()));
    int maxRepeat = quizData.getMaxRepeat();
    int count = jstore.get(oaNumber);
    if ((count == answerHandled)) {
      if (logger.isInfoEnabled()) {
        logger.info("DON'T HANDLE: already handled for abonent: " + oa);
      }
      return null;
    }
    if(logger.isInfoEnabled()) {
      logger.info("maxrepeat: "+ maxRepeat);
      logger.info("count: "+ count);
      logger.info("oa: "+ oa);
    }
    if(count >= maxRepeat) {
      return null;
    }
    ReplyPattern replyPattern = getReplyPattern(text);
    if (replyPattern != null) {
      jstore.put(oaNumber, answerHandled);
      result = new Result(replyPattern.getAnswer(), Result.ReplyRull.OK, quizData.getSourceAddress());
    } else {
      if (maxRepeat > 0) {
        if (count != -1) {
          count++;
          jstore.put(oaNumber, count);
        } else {
          jstore.put(oaNumber, 1);
        }
        result = new Result(quizData.getQuestion(), Result.ReplyRull.REPEAT, quizData.getSourceAddress());
      } else {
        jstore.put(oaNumber, 0);
        result = null;
      }
    }

    try {
      replyStatsDataSource.add(new Reply(new Date(), oa, quizData.getDestAddress(), text));
      if (logger.isInfoEnabled()) {
        logger.info("Sms stored: " + oa + " " + quizData.getDestAddress() + " " + text);
      }
    } catch (ReplyDataSourceException e) {
      logger.error("Can't add reply", e);
      throw new QuizException("Can't add reply", e);
    }
    if ((result != null) && (result.getReplyRull().equals(Result.ReplyRull.REPEAT))) {
      try {
        distributionManager.resend(oa, statusFile.getDistrId());
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
    try {
      exportLock.lock();

      if (exported)
        return;
      exported = true;
      if (logger.isInfoEnabled()) {
        logger.info("Export statistics begining for: " + fileName);
      }
      Date dateBegin = quizData.getDateBegin();
      Date dateEnd = quizData.getDateEnd();
      String da = quizData.getDestAddress();
      Date realStartDate = statusFile.getActualStartDate();
      SimpleDateFormat dateFormat = new SimpleDateFormat("ddMMyy_HHmmss");
      String fileName = dirResult + File.separator + quizId + "." + dateFormat.format(dateBegin) + "-" + dateFormat.format(dateEnd) + ".res";
      dateFormat = new SimpleDateFormat("dd.MM.yy HH:mm:ss");

      File file = new File(fileName);
      if (file.exists()) {
        logger.info("Results already exist for quiz: " + quizId);
        return;
      }
      File fileTmp = new File(file.getAbsolutePath() + "." + dateFormat.format(new Date()));
      File parentFile = file.getParentFile();
      if ((parentFile != null) && (!parentFile.exists())) {
        parentFile.mkdirs();
      }

      PrintWriter printWriter = null;
      String encoding = System.getProperty("file.encoding");
      try {
        printWriter = new PrintWriter(fileTmp, encoding);
        ResultSet resultSet = distributionManager.getStatistics(this.getDistrId(), realStartDate, dateEnd);
        String comma = ",";
        while (resultSet.next()) {
          Reply reply;
          StatsDelivery delivery = (StatsDelivery) resultSet.get();
          String oa = delivery.getMsisdn();
          Date dateDelivery = delivery.getDate();
          if (logger.isInfoEnabled()) {
            logger.info("Analysis delivery: " + delivery);
          }
          if ((reply = replyStatsDataSource.getLastReply(oa, da, realStartDate, dateEnd)) != null) {
            String text = reply.getText();
            ReplyPattern replyPattern = getReplyPattern(text);
            String category;
            if (replyPattern != null) {
              category = replyPattern.getCategory();
            } else {
              String defaultCategory = quizData.getDefaultCategory();
              if (defaultCategory != null) {
                category = defaultCategory;
              } else {
                continue;
              }
            }
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
      } catch (Exception e) {
        logger.error(e, e);
        throw new QuizException(e);
      } finally {
        if (printWriter != null) {
          printWriter.close();
        }
      }
      if (!fileTmp.renameTo(file)) {
        logger.error("Can't rename file: " + fileTmp.getAbsolutePath() + " to " + file.getAbsolutePath());
        throw new QuizException("Can't rename file: " + fileTmp.getAbsolutePath() + " to " + file.getAbsolutePath());
      }
      logger.info("Export statistics finished");
    } finally {
      exportLock.unlock();
    }
  }

  public Date getDateBegin() {
    return quizData.getDateBegin();
  }

  public void setDateBegin(Date dateBegin) throws QuizException {
    if((quizData.getDateBegin() == null)||(!quizData.getDateBegin().equals(dateBegin))) {
      quizData.setDateBegin(dateBegin);
      statusFile.setActualStartDate(dateBegin);
      quizCollector.alert(this);
    }
  }

  public Date getDateEnd() {
    return quizData.getDateEnd();
  }

  public void setDateEnd(Date dateEnd) throws QuizException{
    if((quizData.getDateEnd() == null)||(!quizData.getDateEnd().equals(dateEnd))) {
      quizData.setDateEnd(dateEnd);
      quizCollector.alert(this);
    }
  }

  public String getQuestion() {
    return quizData.getQuestion();
  }

  public void setQuestion(String question) {
    quizData.setQuestion(question);
  }

  public String getFileName() {
    return fileName;
  }

  public void setFileName(String fileName) {
    this.fileName = fileName;
  }

  public String getDestAddress() {
    return quizData.getDestAddress();
  }

  public void setDestAddress(String destAddress) {
    quizData.setDestAddress(destAddress);
  }

  public String getDistrId() {
    return statusFile.getDistrId();
  }

  public void setDistrId(String id) throws QuizException {
    statusFile.setDistrId(id);
  }

  public void addReplyPattern(ReplyPattern replyPattern) {
    if (replyPattern != null) {
      quizData.addReplyPattern(replyPattern);
    }
  }

  public void clearPatterns() {
    quizData.clearPatterns();
  }

  public void setDefaultCategory(String category) {
    if (category != null) {
      quizData.setDefaultCategory(category);
    }
  }

  public void setMaxRepeat(int maxRepeat) {
    quizData.setMaxRepeat(maxRepeat);
  }

  private ReplyPattern getReplyPattern(String text) {
    if (text != null) {
      for (ReplyPattern rP : quizData.getPatterns()) {
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
    strBuilder.append("dest address: ").append(quizData.getDestAddress()).append(sep);
    strBuilder.append("question: ").append(quizData.getQuestion()).append(sep);
    strBuilder.append("dateBegin: ").append(quizData.getDateBegin()).append(sep);
    strBuilder.append("dateEnd: ").append(quizData.getDateEnd()).append(sep);
    strBuilder.append("id: ").append(this.getDistrId()).append(sep);
    strBuilder.append("status: ").append(this.getQuizStatus().toString()).append(sep);
    return strBuilder.substring(0);
  }

  public String getSourceAddress() {
    return quizData.getSourceAddress();
  }

  public void setSourceAddress(String sourceAddress) {
    quizData.setSourceAddress(sourceAddress);
  }

  public boolean isActive() {
    Date now = new Date();
    return now.after(quizData.getDateBegin()) && now.before(quizData.getDateEnd());
  }

  public String getQuizId() {
    return quizId;
  }

  public void setQuizStatus(Status status) throws QuizException {
    if(status != getQuizStatus()) {
      statusFile.setQuizStatus(status);
      quizCollector.alert(this);
    }
  }

  public Status getQuizStatus() {
    return statusFile.getQuizStatus();
  }

  public void setQuizStatus(QuizError error, String reason) throws QuizException {
    if (getQuizStatus() != Status.FINISHED_ERROR) {
      statusFile.setQuizErrorStatus(error, reason);
      quizCollector.alert(this);
    }
  }

  public String getQuizName() {
    return quizData.getQuizName();
  }

  public void setQuizName(String quizName) {
    quizData.setQuizName(quizName);
  }

  public boolean isExported() {
    return exported;
  }

  public String getOrigAbFile() {
    return quizData.getOrigAbFile();
  }

  public void setOrigAbFile(String origAbFile) {
    quizData.setOrigAbFile(origAbFile);
  }

  public Calendar getTimeBegin() {
    return quizData.getTimeBegin();
  }

  public void setTimeBegin(Calendar timeBegin) {
    quizData.setTimeBegin(timeBegin);
  }

  public Calendar getTimeEnd() {
    return quizData.getTimeEnd();
  }

  public void setTimeEnd(Calendar timeEnd) {
    quizData.setTimeEnd(timeEnd);
  }

  public void addDay(Distribution.WeekDays weekDays) {
    quizData.addDay(weekDays);
  }

  public EnumSet<Distribution.WeekDays> getDays() {
    return EnumSet.copyOf(quizData.getDays());
  }

  public boolean isTxmode() {
    return quizData.isTxmode();
  }

  public void setTxmode(boolean txmode) {
    quizData.setTxmode(txmode);
  }

  public Date getDistrDateEnd() {
    return quizData.getDistrDateEnd();
  }

  public void setDistrDateEnd(Date distrDateEnd) {
    quizData.setDistrDateEnd(distrDateEnd);
  }
  public void setReplyPatterns(List<ReplyPattern> replyPatterns) {
    if(replyPatterns==null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    quizData.setReplyPatterns(replyPatterns);
  }

  public void remove() {
    File file = new File(archiveDir);
    if (!file.exists()) {
      file.mkdirs();
    }

    file = new File(getOrigAbFile());
    if (file.exists()) {
      renameFile(new File(getOrigAbFile()));
    } else {
      file = new File(quizDir + File.separator + file.getName());
      renameFile(file);
    }

    String parentSlashQuizId = workDir + File.separator + quizId;

    renameFile(new File(parentSlashQuizId + ".status"));
    deleteFile(new File(parentSlashQuizId + ".xml.bin"));
    deleteFile(new File(parentSlashQuizId + ".xml.bin.j"));
    renameFile(new File(parentSlashQuizId + ".error"));
    file = new File(dirResult);
    File files[] = file.listFiles();
    file = null;
    if (files != null) {
      for (File file1 : files) {
        if ((file1.isFile()) && (file1.getName().startsWith(quizId + "."))) {
          file = file1;
          break;
        }
      }
    }
    if (file != null) {
      renameFile(file);
    }
  }

  private void renameFile(File file) {
    try {
      String name = file.getName();
      file.renameTo(new File(archiveDir + File.separator + name + "." + dateFormat.format(new Date())));
    } catch (Exception e) {
      logger.error(e, e);
    }
  }

  private void deleteFile(File file) {
    try {
      file.delete();
    } catch (Exception e) {
      logger.error(e);
    }
  }

public void writeError(Throwable exc) {
  File file = new File(getFileName());
  String quizName = file.getName().substring(0, file.getName().lastIndexOf("."));
  String errorFile = workDir + File.separator + quizName + ".error";
  PrintWriter writer = null;
  try {
    writer = new PrintWriter(new BufferedWriter(new FileWriter(errorFile, true)));
    writer.println("Error during creating quiz:");
    exc.printStackTrace(writer);
    writer.flush();
  } catch (Exception e) {
    logger.error("Unable to create error file: " + errorFile, e);
  } finally {
    if (writer != null) {
      writer.close();
    }
  }
}

  public void writeQuizesConflict(String newQuizFileName, Quiz prevQuiz) {
    if (newQuizFileName == null) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }
    logger.warn("Conflict quizes");
    File newQuizfile = new File(newQuizFileName);
    String newQuizName = newQuizfile.getName().substring(0, newQuizfile.getName().lastIndexOf("."));
    PrintWriter writer = null;
    String errorFile = workDir + File.separator + newQuizName + ".error";
    try {
      writer = new PrintWriter(new BufferedWriter(new FileWriter(errorFile, true)));
      writer.println(" Quizes conflict:");
      writer.println();
      writer.println("Previous quiz");
      writer.println(prevQuiz.toString());
      writer.println();
      writer.flush();
    } catch (IOException e) {
      logger.error("Unable to create error file: " + errorFile, e);
    } finally {
      if (writer != null) {
        writer.close();
      }
    }
  }

  public void setLastDistrStatusCheck(long l) throws QuizException{
    if(this.lastDistrStatusCheck != l) {
      this.lastDistrStatusCheck = l;
      quizCollector.alert(this);
    }
  }

  public Long getLastDistrStatusCheck(){
    return lastDistrStatusCheck;
  }

  public void updateQuiz(QuizData data) throws QuizException {
    if(data == null) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }
    switch (getQuizStatus()) {
      case NEW:
        if (quizData == null) {
          quizData = data;
          statusFile.setActualStartDate(quizData.getDateBegin());
          break;
        }
      case GENERATION:
      case AWAIT:
        setReplyPatterns(data.getPatterns());
        setDestAddress(data.getDestAddress());
      case ACTIVE:
        setMaxRepeat(data.getMaxRepeat());
        setDefaultCategory(data.getDefaultCategory());
    }
      quizCollector.alert(this);
  }

  public QuizData getQuizData() throws QuizException{
    return quizData;
  }

}