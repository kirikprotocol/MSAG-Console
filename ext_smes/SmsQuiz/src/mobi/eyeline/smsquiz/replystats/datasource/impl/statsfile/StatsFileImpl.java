package mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile;

import com.eyeline.utils.FileUtils;
import com.eyeline.utils.IOUtils;
import com.eyeline.utils.tree.radix.StringsRTree;
import com.eyeline.utils.tree.radix.FileBasedStringsRTree;
import com.eyeline.utils.tree.radix.UnmodifiableRTree;
import com.eyeline.utils.tree.radix.Serializer;

import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import org.apache.log4j.Logger;
import mobi.eyeline.smsquiz.replystats.Reply;

class StatsFileImpl implements StatsFile {
    private static Logger logger = Logger.getLogger(StatsFileImpl.class);

    private ReplyTreeHandler treeHandler;
    private StringsRTree<List<Long>> stringsRTree;

    private final String da;
    private RandomAccessFile randomAccessFile;// private PrintWriter writer;
    private SimpleDateFormat dateFormat;
    private SimpleDateFormat timeFormat;
    private SimpleDateFormat csvDateFormat;
    private String filePath;


    public StatsFileImpl(final String da, final String filePath) throws FileStatsException{
        this(da,filePath, "HH:mm", "yyyyMMdd");
    }
    public StatsFileImpl(final String da, final String filePath, String timePattern, String datePattern) throws FileStatsException {
        this.da = da;
        this.filePath = filePath;

        dateFormat = new SimpleDateFormat(datePattern);
        timeFormat = new SimpleDateFormat(timePattern);
        csvDateFormat = new SimpleDateFormat(datePattern+ ' ' +timePattern);
        File currentFile = new File(filePath);
        File parent = currentFile.getParentFile();
        if (!parent.exists())
            parent.mkdirs();
        else {
            try {
                FileUtils.truncateFile(currentFile,"\n".getBytes()[0],10);
            } catch (IOException e) {
                logger.error("Unable to truncate file",e);
                throw new FileStatsException("Unable to truncate file",e);
            }
        }
    }

    public void open() throws FileStatsException {
        try{
            randomAccessFile = new RandomAccessFile(filePath,"rw");//writer = new PrintWriter(new BufferedWriter(new FileWriter(filePath,true)));
            randomAccessFile.seek(randomAccessFile.length());
        } catch (IOException e) {
            logger.error("Can't create io stream",e);
            throw new FileStatsException("Can't create io stream",e);
        }
        _initTreeHandler();
        if(logger.isInfoEnabled()) {
            logger.info("File: "+filePath+" opened");
        }

    }

    private void _initTreeHandler() throws FileStatsException{
        String truncateName = filePath.substring(0,filePath.lastIndexOf("."));
        String treeFileName = truncateName + ".tree";
        stringsRTree = new StringsRTree<List<Long>>();
        treeHandler = new ReplyTreeHandler(stringsRTree, treeFileName);

        if(new File(treeFileName).exists()) {
            FileBasedStringsRTree<List<Long>> fileBasedStringsRTree = treeHandler.creatFileBasedStringsRTree(treeFileName);
            for(UnmodifiableRTree.Entry<List<Long>> entry:fileBasedStringsRTree.entries()) {
                stringsRTree.put(entry.getKey(),entry.getValue());
            }
            if(logger.isInfoEnabled()) {
                logger.info("Reading tree from file completed: "+treeFileName);
            }
            fileBasedStringsRTree.close();
        } else {
            String msisdn = null;
            try {
                if(randomAccessFile.length()>0) {      // ReplyFile isn't empty, tree file doesn't exist => system crushed previously
                    long prevPosition = randomAccessFile.getFilePointer();
                    randomAccessFile.seek(0);

                    long beginPos = 0;
                    long endPos = 0;

                    int b;
                    while(true) {
                        if((b = randomAccessFile.read()) != -1) {
                            if(b==(int)("\n".getBytes()[0])) {
                                endPos =  randomAccessFile.getFilePointer();
                                if((msisdn = getMsisdn( beginPos, false ))!=null)  {
                                    treeHandler.put(msisdn, beginPos);
                                }
                                beginPos = endPos+1;
                            }
                        } else {
                            break;
                        }
                    }
                    randomAccessFile.seek(prevPosition);
                }
            } catch (IOException e) {
                logger.error("Error during restore tree file",e);
                try {
                    randomAccessFile.close();
                } catch (IOException e1) {
                    logger.error("Can't close io stream",e1);
                }
                throw  new FileStatsException("Error during restore tree file",e);
            }
        }

    }



    public void add(Reply reply)  throws FileStatsException {
        if((reply.getDate()==null)||(reply.getOa()==null)||(reply.getText()==null)) {
            logger.error("Some arguments are missed");
            throw new FileStatsException("Some arguments are missed", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
        }

        try {
            long filePointer = randomAccessFile.getFilePointer();
            randomAccessFile.writeBytes(dateFormat.format(reply.getDate()));
            randomAccessFile.writeBytes(",");
            randomAccessFile.writeBytes(timeFormat.format(reply.getDate()));
            randomAccessFile.writeBytes(",");
            randomAccessFile.writeBytes(reply.getOa());
            randomAccessFile.writeBytes(",");
            randomAccessFile.writeBytes(reply.getText());
            randomAccessFile.writeBytes("\n");
            treeHandler.put(reply.getOa(), filePointer);
        } catch (IOException e) {
            logger.error("Unable to write reply", e);
            try {
                randomAccessFile.close();
            } catch (IOException ex) {
                logger.error("Can't close io stream",ex);
            }
            throw new FileStatsException("Unable to write reply",e);
        }
        /*  writer.print(dateFormat.format(reply.getDate()));
     writer.print(",");
     writer.print(timeFormat.format(reply.getDate()));
     writer.print(",");
     writer.print(reply.getOa());
     writer.print(",");
     writer.println(reply.getText());
     writer.flush();   */
    }
	 

	@SuppressWarnings({"unchecked"})
    public  void list(Date from, Date till, Collection<Reply> result)  throws FileStatsException {
        if(logger.isInfoEnabled()) {
            logger.info("Listing of replies from: "+from+" till: "+till);
        }
        if((result==null)||(from==null)||(till==null)) {
            logger.error("Some arguments are null!");
            throw new FileStatsException("Some arguments are null!", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        String line;
        StringTokenizer tokenizer;
        Date date;
        Reply reply;
        BufferedReader reader = null;
        try {
            reader = new BufferedReader(new FileReader(filePath));
            while((line = reader.readLine())!=null) {
                tokenizer = new StringTokenizer(line,",");
                date = csvDateFormat.parse(tokenizer.nextToken()+" "+tokenizer.nextToken());
                if((date.compareTo(till)<=0)&&(date.compareTo(from)>=0)) {
                    reply = new Reply();
                    reply.setDa(da);
                    reply.setDate(date);
                    reply.setOa(tokenizer.nextToken());
                    reply.setText(tokenizer.nextToken());
                    result.add(reply);
                }
            }

        } catch (FileNotFoundException e) {
            logger.info("Unable to create file reader, maybe file doesn't exist",e);
        }
        catch (NoSuchElementException e) {
            logger.error("Unsupported file format",e);
            throw new FileStatsException("Unsupported file format",e);
        } catch (ParseException e) {
            logger.error("Unsupported file format",e);
            throw new FileStatsException("Unsupported file format",e);
        } catch (IOException e) {
            logger.error("IOException during reading file",e);
            throw new FileStatsException("IOException during reading file",e);
        } finally {
            if(reader!=null) {
                try {
                    reader.close();
                } catch (IOException e) {
                    logger.error("Can't close reader",e);
                    throw new FileStatsException("Can't close reader", e);
                }
            }
        }

    }

    public List<Reply> getReplies(String oa, Date from, Date till) throws FileStatsException {
        if((oa==null)||(from==null)||(till==null)) {
            logger.error("Some arguments are null!");
            throw new FileStatsException("Some arguments are null!", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        List<Long> positions = treeHandler.getValue(oa);
        if(positions==null) {
            return null;
        }
        List<Reply> result = new LinkedList<Reply>();
        try{
            long prevPosition = randomAccessFile.getFilePointer();
            for(Long aPos:positions) {
                result.add(getReply(aPos, false));
            }
            randomAccessFile.seek(prevPosition);
        } catch (IOException e) {
            logger.error("Unable to get Replies from oa: "+oa,e);
            try {
                randomAccessFile.close();
            } catch (IOException e1) {
                logger.error("Can't close io stream",e1);
            }
            throw new FileStatsException("Unable to get Replies from oa: "+oa,e);
        }
        return result;
    }

    public void close() {
        treeHandler.saveTree();
        if(logger.isInfoEnabled()) {
            logger.info("File: "+filePath+" closed");
        }
        if(randomAccessFile!=null) {
            try {
                randomAccessFile.close();
            } catch (IOException e) {
                logger.error("Can't close io stream",e);
            }
        }


     /*   if(writer!=null) {
            writer.close();
        } */
    }

    private Reply getReply(long position, final boolean seekBack) throws FileStatsException{
        long prevPosition = 0;
        Reply reply = new Reply();
        try {
            if(seekBack) {
                prevPosition = randomAccessFile.getFilePointer();
            }

            randomAccessFile.seek(position);
            String line = randomAccessFile.readLine();
            StringTokenizer tokenizer = new StringTokenizer(line,",");

            Date date = csvDateFormat.parse(tokenizer.nextToken()+" "+tokenizer.nextToken());
            reply.setDa(da);
            reply.setDate(date);
            reply.setOa(tokenizer.nextToken());
            reply.setText(tokenizer.nextToken());

            if(seekBack) {
                randomAccessFile.seek(prevPosition);
            }

            return reply;

        } catch (IOException e) {
            logger.error("Unable to get Reply on position: "+position,e);
            try {
                randomAccessFile.close();
            } catch (IOException e1) {
                logger.error("Can't close io stream",e1);
            }
            throw new FileStatsException("Unable to get Reply on position: "+position,e);
        } catch (ParseException e) {
            logger.error("Unable to get Reply on position: "+position,e);
            throw new FileStatsException("Unable to get Reply on position: "+position,e);
        }
    }

    private String getMsisdn(long position, final boolean seekBack) throws FileStatsException{
        long prevPosition = 0;
        try {
            if(seekBack) {
                prevPosition = randomAccessFile.getFilePointer();
            }

            randomAccessFile.seek(position);
            String line = randomAccessFile.readLine();
            if(line==null) {
                System.out.println("Error");
            }
            StringTokenizer tokenizer = new StringTokenizer(line,",");

            tokenizer.nextToken();tokenizer.nextToken();

            if(seekBack) {
                randomAccessFile.seek(prevPosition);
            }

            return tokenizer.nextToken();

        } catch (IOException e) {
            logger.error("Unable to get Reply on position: "+position,e);
            try {
                randomAccessFile.close();
            } catch (IOException e1) {
                logger.error("Can't close io stream",e1);
            }
            throw new FileStatsException("Unable to get Reply on position: "+position,e);
        }
    }



	 
}
 
