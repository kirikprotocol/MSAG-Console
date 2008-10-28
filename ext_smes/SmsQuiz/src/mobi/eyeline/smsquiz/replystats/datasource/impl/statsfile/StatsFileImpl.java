package mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile;

import com.eyeline.utils.FileUtils;
import com.eyeline.utils.tree.radix.StringsRTree;
import com.eyeline.utils.tree.radix.FileBasedStringsRTree;
import com.eyeline.utils.tree.radix.UnmodifiableRTree;

import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import org.apache.log4j.Logger;
import mobi.eyeline.smsquiz.replystats.Reply;

class StatsFileImpl implements StatsFile {
  private static Logger logger = Logger.getLogger(StatsFileImpl.class);

  private StringsRTree<List<Long>> stringsRTree;

  private final String da;
  private RandomAccessFile randomAccessFile;// private PrintWriter writer;
  private SimpleDateFormat dateFormat;
  private SimpleDateFormat timeFormat;
  private SimpleDateFormat csvDateFormat;
  private String filePath;
  private String treeFileName;


  public StatsFileImpl(final String da, final String filePath) throws FileStatsException {
    this(da, filePath, "HH:mm", "yyyyMMdd");
  }

  public StatsFileImpl(final String da, final String filePath, String timePattern, String datePattern) throws FileStatsException {
    this.da = da;
    this.filePath = filePath;

    dateFormat = new SimpleDateFormat(datePattern);
    timeFormat = new SimpleDateFormat(timePattern);
    csvDateFormat = new SimpleDateFormat(datePattern + ' ' + timePattern);
    File currentFile = new File(filePath);
    File parent = currentFile.getParentFile();
    if (!parent.exists())
      parent.mkdirs();
    else {
      try {
        FileUtils.truncateFile(currentFile, System.getProperty("line.separator").getBytes()[0], 10);
      } catch (IOException e) {
        logger.error("Unable to truncate file", e);
        throw new FileStatsException("Unable to truncate file", e);
      }
    }
  }

  public void open() throws FileStatsException {
    try {
      randomAccessFile = new RandomAccessFile(filePath, "rw");//writer = new PrintWriter(new BufferedWriter(new FileWriter(filePath,true)));
      randomAccessFile.seek(randomAccessFile.length());
    } catch (IOException e) {
      logger.error("Can't create io stream", e);
      if (randomAccessFile != null) {
        try {
          randomAccessFile.close();
        } catch (IOException e1) {
          logger.error("Can't close io stream", e1);
        }
      }
      throw new FileStatsException("Can't create io stream", e);
    }
    _initTree();
    if (logger.isInfoEnabled()) {
      logger.info("File: " + filePath + " opened");
    }

  }

  private void _initTree() {
    String truncateName = filePath.substring(0, filePath.lastIndexOf("."));
    treeFileName = truncateName + ".tree";
    stringsRTree = new StringsRTree<List<Long>>();

    if (new File(treeFileName).exists()) {
      FileBasedStringsRTree<List<Long>> fileBasedStringsRTree = new FileBasedStringsRTree<List<Long>>(treeFileName, new LongListSerializer());
      for (UnmodifiableRTree.Entry<List<Long>> entry : fileBasedStringsRTree.entries()) {
        stringsRTree.put(entry.getKey(), entry.getValue());
      }
      fileBasedStringsRTree.close();
      if (logger.isInfoEnabled()) {
        logger.info("Reading tree from file completed: " + treeFileName);
      }
    }

  }

  private boolean notInitTree = true;

  public void add(Reply reply) throws FileStatsException {
    if ((reply.getDate() == null) || (reply.getOa() == null) || (reply.getText() == null)) {
      logger.error("Some arguments are missed");
      throw new FileStatsException("Some arguments are missed", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
    }

    try {
      if (notInitTree) {
        if ((!new File(treeFileName).exists()) && (randomAccessFile.length() > 1)) {      // ReplyFile isn't empty, tree file doesn't exist => system crushed previously
          long prevPosition = randomAccessFile.getFilePointer();
          randomAccessFile.seek(0);

          long beginPos = 0;
          long endPos;
          String msisdn;
          int nByte = (int) (System.getProperty("line.separator").getBytes()[0]);

          int b;
          while ((b = randomAccessFile.read()) != -1) {
            if (b == nByte) {
              endPos = randomAccessFile.getFilePointer();
              if ((msisdn = getMsisdn(beginPos, false)) != null) {
                put(msisdn, beginPos);
              }
              beginPos = endPos + 1;
            }
          }
          randomAccessFile.seek(prevPosition);
        }
        notInitTree = false;
      }
      long filePointer = randomAccessFile.getFilePointer();
      randomAccessFile.writeBytes(dateFormat.format(reply.getDate()));
      randomAccessFile.writeBytes(",");
      randomAccessFile.writeBytes(timeFormat.format(reply.getDate()));
      randomAccessFile.writeBytes(",");
      randomAccessFile.writeBytes(reply.getOa());
      randomAccessFile.writeBytes(",");
      randomAccessFile.writeBytes(reply.getText());
      randomAccessFile.writeBytes(System.getProperty("line.separator"));
      put(reply.getOa(), filePointer);
    } catch (IOException e) {
      logger.error("Unable to write reply", e);
      try {
        randomAccessFile.close();
      } catch (IOException ex) {
        logger.error("Can't close io stream", ex);
      }
      throw new FileStatsException("Unable to write reply", e);
    }
  }


  public List<Reply> getReplies(String oa, Date from, Date till) throws FileStatsException {
    if ((oa == null) || (from == null) || (till == null)) {
      logger.error("Some arguments are null!");
      throw new FileStatsException("Some arguments are null!", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    List<Long> positions = stringsRTree.get(oa);
    if (positions == null) {
      return null;
    }
    List<Reply> result = new LinkedList<Reply>();
    try {
      long prevPosition = randomAccessFile.getFilePointer();
      for (Long aPos : positions) {
        result.add(getReply(aPos, false));
      }
      randomAccessFile.seek(prevPosition);
    } catch (IOException e) {
      logger.error("Unable to get Replies from oa: " + oa, e);
      try {
        randomAccessFile.close();
      } catch (IOException e1) {
        logger.error("Can't close io stream", e1);
      }
      throw new FileStatsException("Unable to get Replies from oa: " + oa, e);
    }
    return result;
  }

  public void close() {
    saveTree();
    if (logger.isInfoEnabled()) {
      logger.info("File: " + filePath + " closed");
    }
    if (randomAccessFile != null) {
      try {
        randomAccessFile.close();
      } catch (IOException e) {
        logger.error("Can't close io stream", e);
      }
    }
  }

  private Reply getReply(long position, final boolean seekBack) throws FileStatsException {
    long prevPosition = 0;
    Reply reply = new Reply();
    try {
      if (seekBack) {
        prevPosition = randomAccessFile.getFilePointer();
      }

      randomAccessFile.seek(position);
      String line = randomAccessFile.readLine();
      StringTokenizer tokenizer = new StringTokenizer(line, ",");

      Date date = csvDateFormat.parse(tokenizer.nextToken() + " " + tokenizer.nextToken());
      reply.setDa(da);
      reply.setDate(date);
      reply.setOa(tokenizer.nextToken());
      String text = tokenizer.nextToken().replaceAll("\\\\n", System.getProperty("line.separator"));
      while (tokenizer.hasMoreTokens()) {
        text += "," + tokenizer.nextToken();
      }
      reply.setText(text);

      if (seekBack) {
        randomAccessFile.seek(prevPosition);
      }

      return reply;

    } catch (IOException e) {
      logger.error("Unable to get Reply on position: " + position, e);
      try {
        randomAccessFile.close();
      } catch (IOException e1) {
        logger.error("Can't close io stream", e1);
      }
      throw new FileStatsException("Unable to get Reply on position: " + position, e);
    } catch (ParseException e) {
      logger.error("Unable to get Reply on position: " + position, e);
      throw new FileStatsException("Unable to get Reply on position: " + position, e);
    }
  }

  private String getMsisdn(long position, final boolean seekBack) throws FileStatsException {
    long prevPosition = 0;
    try {
      if (seekBack) {
        prevPosition = randomAccessFile.getFilePointer();
      }

      randomAccessFile.seek(position);
      String line = randomAccessFile.readLine();
      if (line == null) {
        logger.warn("Unsupported file format");
        return null;
      }
      StringTokenizer tokenizer = new StringTokenizer(line, ",");

      tokenizer.nextToken();
      tokenizer.nextToken();

      if (seekBack) {
        randomAccessFile.seek(prevPosition);
      }

      return tokenizer.nextToken();

    } catch (IOException e) {
      logger.error("Unable to get Reply on position: " + position, e);
      try {
        randomAccessFile.close();
      } catch (IOException e1) {
        logger.error("Can't close io stream", e1);
      }
      throw new FileStatsException("Unable to get Reply on position: " + position, e);
    }
  }

  private void put(String key, long value) throws FileStatsException {
    if (logger.isInfoEnabled()) {
      logger.info("Put into tree entry: " + key + " " + value);
    }
    if (key == null) {
      logger.error("Some arguments are null");
      throw new FileStatsException("Some arguments are null", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    List<Long> previousValue;
    List<Long> nextValue;
    if ((previousValue = stringsRTree.get(key)) != null) {
      nextValue = new LinkedList<Long>(previousValue);
    } else {
      nextValue = new LinkedList<Long>();
    }
    nextValue.add(value);
    stringsRTree.put(key, nextValue);
  }

  private void saveTree() {
    File file = new File(treeFileName);
    file.delete();
    FileBasedStringsRTree.createRTree(stringsRTree, treeFileName, new LongListSerializer());
    logger.info("Tree saved");
  }

}
 
