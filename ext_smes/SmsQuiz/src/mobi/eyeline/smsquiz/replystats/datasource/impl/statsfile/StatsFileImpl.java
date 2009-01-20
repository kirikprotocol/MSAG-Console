package mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile;

import com.eyeline.utils.FileUtils;
import com.eyeline.utils.tree.radix.FileBasedStringsRTree;
import com.eyeline.utils.tree.radix.StringsRTree;
import com.eyeline.utils.tree.radix.UnmodifiableRTree;
import mobi.eyeline.smsquiz.replystats.Reply;
import org.apache.log4j.Logger;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import java.util.StringTokenizer;

@SuppressWarnings({"ResultOfMethodCallIgnored"})
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
  private String encoding;


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
    encoding = System.getProperty("file.encoding");

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
              if ((msisdn = getMsisdn(beginPos)) != null) {
                put(msisdn, beginPos);
              }
              beginPos = endPos + 1;
            }
          }
          randomAccessFile.seek(prevPosition);
        }
        notInitTree = false;
      }
      byte[] comma = ",".getBytes();
      long filePointer = randomAccessFile.getFilePointer();
      randomAccessFile.write(dateFormat.format(reply.getDate()).getBytes(encoding));
      randomAccessFile.write(comma);
      randomAccessFile.write(timeFormat.format(reply.getDate()).getBytes(encoding));
      randomAccessFile.write(comma);
      randomAccessFile.write(reply.getOa().getBytes(encoding));
      randomAccessFile.write(comma);
      randomAccessFile.write(reply.getText().getBytes(encoding));
      randomAccessFile.write(System.getProperty("line.separator").getBytes(encoding));
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


  public List<Reply> getReplies(String oa, final Date from, final Date till) throws FileStatsException {
    if ((oa == null) || (from == null) || (till == null)) {
      logger.error("Some arguments are null!");
      throw new FileStatsException("Some arguments are null!", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
    }

    String oa2;
    if (oa.startsWith("+7")) {
      oa2 = "8" + oa.substring(2);
    } else {
      oa2 = "+7" + oa.substring(1);
    }

    List<Long> positions = stringsRTree.get(oa);
    if (positions == null) {
      positions = stringsRTree.get(oa2);
      if (positions == null) {
        return null;
      }
    }
    List<Reply> result = new LinkedList<Reply>();
    try {
      long prevPosition = randomAccessFile.getFilePointer();
      for (Long aPos : positions) {
        Reply reply = getReply(aPos);
        if ((reply != null) && (reply.getDate().compareTo(till) <= 0) && (reply.getDate().compareTo(from) >= 0)) {
          result.add(reply);
        }
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

  private Reply getReply(long position) throws FileStatsException {
    Reply reply = new Reply();
    try {
      randomAccessFile.seek(position);
      ByteArrayOutputStream str = new ByteArrayOutputStream();
      int c = -1;
      boolean eol = false;

      while (!eol) {
        switch (c = randomAccessFile.read()) {
          case -1:
          case '\n':
            eol = true;
            break;
          case '\r':
            eol = true;
            long cur = randomAccessFile.getFilePointer();
            if ((randomAccessFile.read()) != '\n')
              randomAccessFile.seek(cur);
            break;
          default:
            str.write(c);
            break;
        }
      }

      if ((c == -1) && (str.size() == 0)) {
        throw new FileStatsException("Index file damaged. Reply not found at position: " + position);
      }
      String line = str.toString(encoding);
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

  private String getMsisdn(long position) throws FileStatsException {
    try {
      randomAccessFile.seek(position);
      String line = randomAccessFile.readLine();
      if (line == null) {
        logger.warn("Unsupported file format");
        return null;
      }
      StringTokenizer tokenizer = new StringTokenizer(line, ",");

      tokenizer.nextToken();
      tokenizer.nextToken();

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
    if (logger.isDebugEnabled()) {
      logger.debug("Put into tree entry: " + key + " " + value);
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

  public String getName() {
    return filePath;
  }

}
 
