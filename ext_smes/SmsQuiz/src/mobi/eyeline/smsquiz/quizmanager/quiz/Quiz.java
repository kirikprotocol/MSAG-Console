package mobi.eyeline.smsquiz.quizmanager.quiz;

import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;

import mobi.eyeline.smsquiz.replystats.datasource.ReplyStatsDataSource;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyDataSourceException;
import mobi.eyeline.smsquiz.replystats.Reply;
import mobi.eyeline.smsquiz.quizmanager.Result;
import mobi.eyeline.smsquiz.quizmanager.QuizException;
import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.distribution.DistributionException;
import mobi.eyeline.smsquiz.distribution.StatsDelivery;
import mobi.eyeline.smsquiz.storage.ResultSet;
import mobi.eyeline.smsquiz.storage.StorageException;
import com.eyeline.jstore.JStore;
import org.apache.log4j.Logger;

/**
 * author: alkhal
 */

public class Quiz {
    private static Logger logger = Logger.getLogger(Quiz.class);

    private String destAddress; //At this number come reply 
	private JStore jstore;
    private String question;
	private String fileName;
	private Date dateBegin;
	private Date dateEnd;

    private int maxRepeat;
    private String defaultCategory;
    private String dirResult;

    private List<ReplyPattern> replyPatterns;

	private ReplyStatsDataSource replyStatsDataSource;
    private DistributionManager distributionManager;

    private Status status;

    public Quiz(final String statusDir, final File file,
                    final ReplyStatsDataSource replyStatsDataSource,
                    final DistributionManager distributionManager, final String dirResult) throws QuizException {
        this.dirResult = dirResult;
        this.replyStatsDataSource = replyStatsDataSource;
        this.distributionManager = distributionManager;
        jstore = new JStore(-1);

        if(file==null) {
            logger.error("Some arguments are null");
            throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        fileName = file.getName();
        String quizName = fileName.substring(0,fileName.lastIndexOf("."));
        jstore.init(quizName+".bin",60000,10);
        status = new Status(statusDir+"/"+quizName+".status");
        replyPatterns = new ArrayList<ReplyPattern>();
    }

    public Result handleSms(String oa, String text) throws QuizException{
        Result result = null;
        int oaNumber = Integer.parseInt( oa.substring( oa.lastIndexOf("+")+1, oa.length() ) );
        ReplyPattern replyPattern = getReplyPattern(text);
        if(replyPattern!=null) {
            if(jstore.get(oaNumber)!=-1) {
                jstore.remove(oaNumber);
            }
            result =  new Result(replyPattern.getAnswer(), Result.ReplyRull.OK);

        } else {
            if(maxRepeat>0) {
                int count;
                if((count = jstore.get(oaNumber))!=-1) {
                    if(count>=maxRepeat) {
                        result = null;
                    }
                    else {
                        count++;
                        jstore.put(oaNumber,count);
                        result = new Result(question, Result.ReplyRull.REPEAT);
                    }
                }
                else {
                    jstore.put(oaNumber,1);
                    result = new Result(question, Result.ReplyRull.REPEAT);
                }
            }
            else {
                result =  null;
            }
        }

        try {
            replyStatsDataSource.add(new Reply(new Date(),oa, destAddress,text));
        } catch (ReplyDataSourceException e) {
            logger.error("Can't add reply", e);
            throw new QuizException("Can't add reply", e);
        }

        return result;

    }

	public void exportStats() throws QuizException{
        SimpleDateFormat dateFormat = new SimpleDateFormat("ddMMyy_HHmmss");
        String fileName = dirResult+"/"+this.getId()+"."+dateFormat.format(dateBegin)+"-"+dateFormat.format(dateEnd)+".res";
        dateFormat = new SimpleDateFormat("dd.MM.yy HH:mm:ss");

        File file = new File(fileName);
        File parentFile = file.getParentFile();
        if((parentFile!=null)&&(!parentFile.exists())) {
            parentFile.mkdirs();
        }
        PrintWriter printWriter = null;
        try {
            printWriter = new PrintWriter(new BufferedWriter(new FileWriter(file)));
            ResultSet resultSet = distributionManager.getStatistics(this.getId(),dateBegin,dateEnd);
            Map<String,Reply> replies = replyStatsDataSource.listMap(destAddress,dateBegin,dateEnd);
            while(resultSet.next()) {
                Reply reply = null;
                StatsDelivery delivery = (StatsDelivery)resultSet.get();
                String oa = delivery.getMsisdn();
                Date dateDelivery = delivery.getDate();
                if((reply = replies.get(oa))!=null) {
                    if((reply.getOa().equals(oa))&&(reply.getDa().equals(destAddress))) {
                        String text = reply.getText();
                        ReplyPattern replyPattern = getReplyPattern(text);
                        String category = null;
                        if(replyPattern!=null) {
                            category = replyPattern.getCategory();
                        } else {
                            if (defaultCategory!=null) {
                                category = defaultCategory;
                            }
                            else {
                                continue;
                            }
                        }                        
                        printWriter.print(oa);
                        printWriter.print(",");
                        printWriter.print(dateFormat.format(dateDelivery));
                        printWriter.print(",");
                        printWriter.print(dateFormat.format(reply.getDate()));
                        printWriter.print(",");
                        printWriter.print(dateFormat.format(reply.getDate()));
                        printWriter.print(",");
                        printWriter.print(category);
                        printWriter.print(",");
                        printWriter.println(reply.getText());
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
            if(printWriter!=null) {
                printWriter.close();
            }
        }
    }

    public Status getStatus() {
		return null;
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

    public void setId(String id) throws QuizException{
        status.setId(id);
    }

    public void addReplyPattern(ReplyPattern replyPattern) {
        if(replyPattern!=null) {
            replyPatterns.add(replyPattern);
        }
    }
    public void clearPatterns() {
        replyPatterns.clear();
    }

    public void setDefaultCategory(String category) {
        if(category!=null) {
            defaultCategory = category;
        }
    }

    public void setMaxRepeat(int maxRepeat) {
        this.maxRepeat = maxRepeat;
    }

    private ReplyPattern getReplyPattern(String text) {
        if(text!=null) {
            for(ReplyPattern rP: replyPatterns) {
                if(rP.matches(text)) {
                    return rP;
                }
            }
        }
        return null;

    }

    public String toString() {
        StringBuilder strBuilder= new StringBuilder();
        strBuilder.append("fileName: ").append(fileName).append("\n");
        strBuilder.append("address: ").append(destAddress).append("\n");
        strBuilder.append("question: ").append(question).append("\n");
        strBuilder.append("dateBegin: ").append(dateBegin).append("\n");
        strBuilder.append("dateEnd: ").append(dateEnd).append("\n");
        strBuilder.append("id: ").append(this.getId()).append("\n");
        return  strBuilder.substring(0);
    }
}