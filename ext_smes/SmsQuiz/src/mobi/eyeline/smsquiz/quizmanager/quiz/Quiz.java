package mobi.eyeline.smsquiz.quizmanager.quiz;

import com.eyeline.jstore.JStore;
import mobi.eyeline.smsquiz.distribution.Distribution;
import mobi.eyeline.smsquiz.distribution.DistributionException;
import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.distribution.StatsDelivery;
import mobi.eyeline.smsquiz.quizmanager.DistributionImpl;
import mobi.eyeline.smsquiz.quizmanager.QuizCollector;
import mobi.eyeline.smsquiz.quizmanager.QuizException;
import mobi.eyeline.smsquiz.quizmanager.Result;
import mobi.eyeline.smsquiz.replystats.Reply;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyDataSourceException;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyStatsDataSource;
import mobi.eyeline.smsquiz.storage.ResultSet;
import mobi.eyeline.smsquiz.subscription.SubscriptionManager;
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

  private QuizData quizData = new QuizData();

  private long lastDistrStatusCheck = System.currentTimeMillis();

  private String dirResult;
  private String archiveDir;
  private String quizDir;
  private String workDir;
  private String quizId;

  private final ReplyStatsDataSource rds;
  private final DistributionManager dm;
  private final SubscriptionManager sm;
  private final QuizCollector quizCollector;

  private int answerHandled = -1;

  private StatusFile statusFile;

  private boolean exported = false;

  private boolean distrGenerated = false;

  private final Lock lock = new ReentrantLock();

  private SimpleDateFormat dateFormat = new SimpleDateFormat("ddMMyy_HHmmss");


  public Quiz(final File file, final ReplyStatsDataSource rds, final QuizCollector quizCollector,
              final DistributionManager dm, final SubscriptionManager sm, final String dirResult,
              final String dirWork, final String archiveDir, final String quizDir, final QuizData quizData) throws QuizException {

    if ((file == null) || (rds == null) || (quizCollector == null) || (dm == null) || (sm == null) ||
        (dirResult == null) || (dirWork == null) || (archiveDir == null) || (quizDir == null) || (quizData == null)) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }
    this.sm = sm;
    this.quizData = quizData;
    this.dirResult = dirResult;
    this.archiveDir = archiveDir;
    this.quizDir = quizDir;
    this.workDir = dirWork;
    this.rds = rds;
    this.dm = dm;
    this.quizCollector = quizCollector;
    jstore = new JStore(0);
    fileName = file.getAbsolutePath();
    quizId = file.getName().substring(0, file.getName().lastIndexOf("."));
    jstore.init(dirWork + File.separator + file.getName() + ".bin", 60000, 10);
    statusFile = new StatusFile(dirWork + File.separator + quizId + ".status");
    setActualStartDate(quizData.getDateBegin());
  }

  public String getStatusFileName() {
    return statusFile.getStatusFileName();
  }

  public synchronized Result handleSms(String oa, String text) throws QuizException {
    if ((exported) || (!statusFile.getQuizStatus().equals(Status.ACTIVE))) {
      return null;
    }
    Result result = null;
    long oaNumber = Long.parseLong(oa.substring(oa.lastIndexOf("+") + 2, oa.length()));
    int maxRepeat = quizData.getMaxRepeat();
    int count = jstore.get(oaNumber);
    if (count == answerHandled) {
      if (logger.isInfoEnabled()) {
        logger.info("DON'T HANDLE: already handled for abonent: " + oa);
      }
      return null;
    }
    if (count > maxRepeat) {
      return null;
    }
    ReplyPattern replyPattern = getReplyPattern(text);
    if (replyPattern != null) {
      jstore.put(oaNumber, answerHandled);
      result = new Result(replyPattern.getAnswer(), Result.ReplyRull.OK, quizData.getSourceAddress());
    } else {
      count++;
      jstore.put(oaNumber, count);
      if (count <= maxRepeat) {
        result = new Result(quizData.getRepeatQuestion(), Result.ReplyRull.REPEAT, quizData.getSourceAddress());
      }
    }
    try {
      rds.add(new Reply(new Date(), oa, quizData.getDestAddress(), text));
      if (logger.isInfoEnabled()) {
        logger.info("Sms stored: " + oa + " " + quizData.getDestAddress() + " " + text);
      }
    } catch (ReplyDataSourceException e) {
      logger.error("Can't add reply", e);
      throw new QuizException("Can't add reply", e);
    }
    if ((result != null) && (result.getReplyRull().equals(Result.ReplyRull.REPEAT))) {
      try {
        dm.resend(oa, getRepeatQuestion(), statusFile.getDistrId());
      } catch (DistributionException e) {
        e.printStackTrace();
        logger.error("Can't resend the message", e);
        throw new QuizException("Can't resend the message", e);
      }
    }
    return result;

  }

  public synchronized void shutdown() {
    jstore.shutdown();
  }

  public void exportStats() throws QuizException {
    if (exported)
      return;
    try {
      lock.lock();
      if (exported)
        return;
      exported = true;
      if (logger.isInfoEnabled()) {
        logger.info("Export statistics begining for: " + fileName);
      }
      String distrId = getDistrId();
      if (distrId == null) {
        logger.error("Distribution id doesn't exists for quiz: " + this);
        throw new QuizException("Distribution id doesn't exists for quiz: " + this);
      }
      Date dateBegin = quizData.getDateBegin();
      Date dateEnd = quizData.getDateEnd();
      String da = quizData.getDestAddress();
      Date realStartDate = statusFile.getActualStartDate();
      SimpleDateFormat dateFormat = new SimpleDateFormat("ddMMyy_HHmmss");
      String results = dirResult + File.separator + quizId + "." + dateFormat.format(dateBegin) + "-" + dateFormat.format(dateEnd) + ".res";
      dateFormat = new SimpleDateFormat("dd.MM.yy HH:mm:ss");

      File resultFile = new File(results);
      if (resultFile.exists()) {
        logger.info("Results already exist for quiz: " + quizId);
        return;
      }
      File resultsTmp = new File(resultFile.getAbsolutePath() + "." + dateFormat.format(new Date()));
      String fileTmp = resultsTmp + ".tmp";
      File parentFile = resultFile.getParentFile();
      if ((parentFile != null) && (!parentFile.exists())) {
        parentFile.mkdirs();
      }
      PrintWriter tmpWriter = null;
      String encoding = System.getProperty("file.encoding");
      try {
        tmpWriter = new PrintWriter(fileTmp, encoding);
        ResultSet resultSet = dm.getStatistics(distrId, realStartDate, dateEnd);
        String comma = ",";
        while (resultSet.next()) {
          Reply reply;
          StatsDelivery delivery = (StatsDelivery) resultSet.get();
          String oa = delivery.getMsisdn();
          Date dateDelivery = delivery.getDate();
          if (logger.isDebugEnabled()) {
            logger.debug("Analysis delivery: " + delivery);
          }
          if ((reply = rds.getLastReply(oa, da, realStartDate, dateEnd)) != null) {
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
            tmpWriter.print(oa);
            StringBuilder answer = new StringBuilder();
            answer.append(comma).append(dateFormat.format(dateDelivery))
                .append(comma).append(dateFormat.format(reply.getDate()))
                .append(comma).append(category).append(comma)
                .append(text.replace(System.getProperty("line.separator"), "\\n"));
            tmpWriter.println(answer);
            if (logger.isDebugEnabled()) {
              logger.debug("Last reply for oa=" + oa + " is " + reply + " stored");
            }
          } else {
            if (logger.isDebugEnabled()) {
              logger.debug("Last reply for oa=" + oa + " is empty");
            }
          }
        }
        tmpWriter.close();
        writeUniq(fileTmp, resultsTmp);
        new File(fileTmp).delete();
      } catch (Exception e) {
        logger.error(e, e);
        throw new QuizException(e);
      } finally {
        if (tmpWriter != null) {
          tmpWriter.close();
        }
      }
      if (!resultsTmp.renameTo(resultFile)) {
        logger.error("Can't rename file: " + resultsTmp.getAbsolutePath() + " to " + resultFile.getAbsolutePath());
        throw new QuizException("Can't rename file: " + resultsTmp.getAbsolutePath() + " to " + resultFile.getAbsolutePath());
      }
      logger.info("Export statistics finished");
    } finally {
      lock.unlock();
    }
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  private void writeUniq(String src, File dest) throws QuizException {
    if (logger.isInfoEnabled()) {
      logger.info("Create uniq records...");
    }
    String encoding = System.getProperty("file.encoding");
    BufferedReader reader = null;
    PrintWriter tmpWriter = null;
    PrintWriter resultsWriter = null;
    String tmpFileName = src + ".tmp";
    File tmpFile = new File(tmpFileName);
    int initialCapacity = 50000; //todo
    try {
      boolean repeat;
      HashMap<String, String> answers = new HashMap<String, String>(initialCapacity);
      resultsWriter = new PrintWriter(dest, encoding);
      int i = 0;
      do {
        repeat = false;
        if (logger.isInfoEnabled()) {
          i++;
        }
        answers.clear();
        reader = new BufferedReader(new FileReader(src));
        tmpWriter = new PrintWriter(tmpFileName, encoding);
        String line;
        while ((line = reader.readLine()) != null) {
          int commaIndex = line.indexOf(",");
          String oa = line.substring(0, commaIndex);
          String answer = line.substring(commaIndex);
          if (answers.size() <= initialCapacity) {
            answers.put(oa, answer);
          } else {
            if (!answers.containsKey(oa)) {
              repeat = true;
              tmpWriter.print(oa);
              tmpWriter.println(answer);
            }
          }
        }
        for (Map.Entry<String, String> e : answers.entrySet()) {
          resultsWriter.print(e.getKey());
          resultsWriter.println(e.getValue());
        }
        resultsWriter.flush();
        tmpWriter.close();
        reader.close();
        String tmpStr = src;
        src = tmpFileName;
        tmpFileName = tmpStr;
      } while (repeat);
      tmpFile.delete();
      if (logger.isInfoEnabled()) {
        logger.info("Creating done");
      }
    } catch (Exception e) {
      logger.error(e, e);
      throw new QuizException(e);
    } finally {
      if (reader != null) {
        try {
          reader.close();
        }
        catch (Exception e) {
        }
      }
      if (tmpWriter != null) {
        tmpWriter.close();
      }
      if (resultsWriter != null) {
        resultsWriter.close();
      }
    }
  }

  public Date getDateBegin() {
    return quizData.getDateBegin();
  }

  public void setDateBegin(Date dateBegin) throws QuizException {
    if ((quizData.getDateBegin() == null) || (!quizData.getDateBegin().equals(dateBegin))) {
      quizData.setDateBegin(dateBegin);
      setActualStartDate(dateBegin);
      quizCollector.alert();
    }
  }

  public Date getDateEnd() {
    return quizData.getDateEnd();
  }

  public void setDateEnd(Date dateEnd) throws QuizException {
    if ((quizData.getDateEnd() == null) || (!quizData.getDateEnd().equals(dateEnd))) {
      quizData.setDateEnd(dateEnd);
      quizCollector.alert();
    }
  }

  public String getQuestion() {
    return quizData.getQuestion();
  }

  public String getRepeatQuestion() {
    return quizData.getRepeatQuestion();
  }

  public void setQuestion(String question) {
    quizData.setQuestion(question);
  }

  public void setRepeatQuestion(String repeatQuestion) {
    quizData.setRepeatQuestion(repeatQuestion);
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

  private void setActualStartDate(Date date) throws QuizException {
    Date prev = statusFile.getActualStartDate();
    if (prev == null) {
      Date now = new Date();
      if (now.before(date)) {
        statusFile.setActualStartDate(date);
      } else {
        statusFile.setActualStartDate(now);
      }
    }
  }

  public String toString() {
    String sep = System.getProperty("line.separator");
    StringBuilder strBuilder = new StringBuilder();
    strBuilder.append(sep);
    strBuilder.append("fileName: ").append(fileName).append(sep);
    if (quizData != null) {
      strBuilder.append("dest address: ").append(quizData.getDestAddress()).append(sep);
      strBuilder.append("question: ").append(quizData.getQuestion()).append(sep);
      strBuilder.append("dateBegin: ").append(quizData.getDateBegin()).append(sep);
      strBuilder.append("dateEnd: ").append(quizData.getDateEnd()).append(sep);
    }
    strBuilder.append("distrId: ").append(this.getDistrId()).append(sep);
    Status status = getQuizStatus();
    if (status != null) {
      strBuilder.append("status: ").append(status.toString()).append(sep);
    }
    return strBuilder.substring(0);
  }

  public String getSourceAddress() {
    return quizData.getSourceAddress();
  }

  public void setSourceAddress(String sourceAddress) {
    quizData.setSourceAddress(sourceAddress);
  }

  public String getQuizId() {
    return quizId;
  }

  public void setQuizStatus(Status status) throws QuizException {
    if (status != statusFile.getQuizStatus()) {
      statusFile.setQuizStatus(status);
      quizCollector.alert();
    }
  }

  public Status getQuizStatus() {
    return statusFile.getQuizStatus();
  }

  public void setQuizStatus(QuizError error, String reason) throws QuizException {
    if (statusFile.getQuizStatus() != Status.FINISHED_ERROR) {
      statusFile.setQuizErrorStatus(error, reason);
      quizCollector.alert();
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
    if (replyPatterns == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    quizData.setReplyPatterns(replyPatterns);
  }

  public synchronized void remove() {
    File file = new File(archiveDir);
    if (!file.exists()) {
      file.mkdirs();
    }
    String orgiAbFile = getOrigAbFile();
    if (orgiAbFile != null) {
      file = new File(orgiAbFile);
      if (file.exists()) {
        renameFile(new File(orgiAbFile));
      } else {
        file = new File(quizDir + File.separator + file.getName());
        renameFile(file);
      }
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
      logger.error(e, e);
    }
  }

  public synchronized void writeError(Throwable exc) {
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

  public synchronized void writeQuizesConflict(String newQuizFileName) {
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
      writer.println(toString());
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

  public void setLastDistrStatusCheck(long l) throws QuizException {
    if (this.lastDistrStatusCheck != l) {
      this.lastDistrStatusCheck = l;
      quizCollector.alert();
    }
  }

  public Long getLastDistrStatusCheck() {
    return lastDistrStatusCheck;
  }

  public void updateQuiz(QuizData data) throws QuizException {
    try {
      lock.lock();
      if (data == null) {
        logger.error("Some arguments are null");
        throw new IllegalArgumentException("Some arguments are null");
      }

      boolean alertDistribution = false;

      switch (statusFile.getQuizStatus()) {
        case NEW:
          if (quizData.getQuizName() == null) {       // quizData didn't init yet
            quizData = data;
            setActualStartDate(quizData.getDateBegin());
            break;
          }
        case GENERATION:
        case AWAIT:
          setDestAddress(data.getDestAddress());
        case ACTIVE:
          setReplyPatterns(data.getPatterns());
          setMaxRepeat(data.getMaxRepeat());
          setDefaultCategory(data.getDefaultCategory());

          if (!data.getDateEnd().equals(getDateEnd())) {
            setDateEnd(data.getDateEnd());
            alertDistribution = true;
          }
          if (!data.getQuestion().equals(getQuestion())) {        //distribution properties
            setQuestion(data.getQuestion());
            alertDistribution = true;
          }
          if (!data.getRepeatQuestion().equals(getRepeatQuestion())) {        //distribution properties
            setRepeatQuestion(data.getRepeatQuestion());
            alertDistribution = true;
          }
          if (!data.getDistrDateEnd().equals(getDistrDateEnd())) {
            setDistrDateEnd(data.getDistrDateEnd());
            alertDistribution = true;
          }
          if (!data.getSourceAddress().equals(getSourceAddress())) {
            setSourceAddress(data.getSourceAddress());
            alertDistribution = true;
          }
          if (data.getTimeBegin().getTimeInMillis() != getTimeBegin().getTimeInMillis()) {
            setTimeBegin(data.getTimeBegin());
            alertDistribution = true;
          }
          if (data.getTimeEnd().getTimeInMillis() != getTimeEnd().getTimeInMillis()) {
            setTimeEnd(data.getTimeEnd());
            alertDistribution = true;
          }
          if (data.isTxmode() != isTxmode()) {
            setTxmode(data.isTxmode());
            alertDistribution = true;
          }
          if (data.getDays().size() != getDays().size()) {
            quizData.setDays(data.getDays());
            alertDistribution = true;
          } else {
            for (Distribution.WeekDays day1 : data.getDays()) {
              boolean flag = false;
              for (Distribution.WeekDays day2 : getDays()) {
                if (day1 == day2) {
                  flag = true;
                  break;
                }
              }
              if (!flag) {
                alertDistribution = true;
                quizData.setDays(data.getDays());
                break;
              }
            }
          }
      }
      quizCollector.alert();
      if (alertDistribution) {
        if (getDistrId() == null) {
          logger.error("Distribution doesn't exist for quiz: " + this + "\n It can't be alerted");
          throw new QuizException("Distribution doesn't exist for quiz: " + this + "\n It can't be alerted");
        }
        try {
          dm.alterDistribution(buildDistribution(), getDistrId());
        } catch (DistributionException e) {
          throw new QuizException(e);
        }
      }
    } catch (Exception e) {
      logger.error(e, e);
      throw new QuizException(e);
    } finally {
      lock.unlock();
    }
  }

  public QuizData getQuizData() {
    return quizData;
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  public void createDistribution() throws QuizException {
    if (distrGenerated) {
      return;
    }
    try {
      lock.lock();
      if (distrGenerated) {
        return;
      }
      distrGenerated = true;
      if (logger.isInfoEnabled()) {
        logger.info("Create distribution for quiz: " + quizId + "...");
      }
      String id;
      DistributionManager.State state;
      boolean create = true;
      if ((id = getDistrId()) != null) {
        if (logger.isInfoEnabled()) {
          logger.info("Quiz will be repaired: " + this);
        }
        state = dm.getState(id);
        if (!state.equals(DistributionManager.State.ERROR)) {
          create = false;
        }
      }
      if (create) {
        Distribution distr = buildDistribution();
        id = dm.createDistribution(distr);
      }

      setDistrId(id);
      if (logger.isInfoEnabled()) {
        logger.info("Distribution created for: " + this);
      }
    } catch (Throwable e) {
      try {
        setQuizStatus(QuizError.DISTR_ERROR, "Error during creating distribution");
      } catch (QuizException ex) {
      }
      writeError(e);
      logger.error(e, e);
      throw new QuizException(e.toString(), e);
    } finally {
      lock.unlock();
    }
  }

  private Distribution buildDistribution() throws QuizException {
    Distribution distr = new DistributionImpl(quizData.getOrigAbFile(), sm);
    distr.setQuestion(quizData.getQuestion());
    distr.setDateBegin(quizData.getDateBegin());
    distr.setDateEnd(quizData.getDistrDateEnd());
    distr.setSourceAddress(quizData.getSourceAddress());
    distr.setTaskName(quizData.getQuizName() + "(SmsQuiz)");
    distr.setTimeBegin(quizData.getTimeBegin());
    distr.setTimeEnd(quizData.getTimeEnd());
    distr.setTxmode(quizData.isTxmode());
    distr.addDays(quizData.getDays());
    return distr;
  }

  public boolean isDistrGenerated() {
    return distrGenerated;
  }
}