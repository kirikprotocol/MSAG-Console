package mobi.eyeline.smsquiz.quizmanager.quiz;

import java.util.Date;
import java.util.List;
import java.util.ArrayList;
import java.io.*;

import mobi.eyeline.smsquiz.replystats.datasource.ReplyStatsDataSource;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyDataSourceException;
import mobi.eyeline.smsquiz.replystats.Reply;
import mobi.eyeline.smsquiz.quizmanager.Result;
import mobi.eyeline.smsquiz.quizmanager.QuizException;
import com.eyeline.jstore.JStore;
import org.apache.log4j.Logger;

/**
 * author: alkhal
 */

public class Quiz {
    private static Logger logger = Logger.getLogger(Quiz.class);

    private String address;  
	private JStore jstore;
    private String question;
	private String fileName;
	private Date dateBegin;
	private Date dateEnd;
    private int maxRepeat;
    private String defaultCategory;

    private List<ReplyPattern> replyPatterns;

	private ReplyStatsDataSource replyStatsDataSource;

	private Status status;

    public Quiz(final String statusDir, final File file, final ReplyStatsDataSource replyStatsDataSource) throws QuizException {
        this.replyStatsDataSource = replyStatsDataSource;
        jstore = new JStore(-1);
        jstore.init("store.bin",10000,10);

        if(file==null) {
            logger.error("Some arguments are null");
            throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        fileName = file.getName();
        String quizName = fileName.substring(0,fileName.lastIndexOf("."));
        status = new Status(statusDir+"/"+quizName+".status");
        replyPatterns = new ArrayList<ReplyPattern>();
    }

    public Result handleSms(String oa, String text) throws ReplyDataSourceException{
        ReplyPattern replyPattern = null;
        int oaNumber = Integer.parseInt( oa.substring ( oa.lastIndexOf("+"), oa.length() ) );
        for(ReplyPattern rP: replyPatterns) {
            if(rP.matches(text)) {
                replyPattern = rP;
                break;
            }
        }
        if(replyPattern!=null) {
            if(jstore.get(oaNumber)!=-1) {
                jstore.remove(oaNumber);
            }
            try {
                replyStatsDataSource.add(new Reply(new Date(),oa, address,replyPattern.getCategory()));
            } catch (ReplyDataSourceException e) {
                logger.error("Can't add reply", e);
                throw new ReplyDataSourceException("Can't add reply", e);
            }
            return new Result(replyPattern.getAnswer(), Result.ReplyRull.OK);
        } else {
            if(maxRepeat>0) {
                int count;
                if((count = jstore.get(oaNumber))!=-1) {
                    if(count>=maxRepeat) {
                        jstore.remove(oaNumber);
                        if(defaultCategory!=null) {
                            try {
                                replyStatsDataSource.add(new Reply( new Date(),oa, address, defaultCategory ));
                            } catch (ReplyDataSourceException e) {
                                logger.error("Can't add reply", e);
                                throw new ReplyDataSourceException("Can't add reply", e);}
                        }
                        return null;
                    }
                    else {
                        count++;
                        jstore.put(oaNumber,count);
                        return new Result(question, Result.ReplyRull.REPEAT);
                    }
                }
                else {
                    jstore.put(oaNumber,1);
                    return new Result(question, Result.ReplyRull.REPEAT);
                }
            }
            else {
                try {
                    replyStatsDataSource.add(new Reply( new Date(),oa, address, defaultCategory ));
                } catch (ReplyDataSourceException e) {
                    logger.error("Can't add reply", e);
                    throw new ReplyDataSourceException("Can't add reply", e);
                }
                return null;
            }
        }

	}

	public void exportStats(String fileName) {
        //todo
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

    public String getAddress() {
        return address;
    }

    public void setAddress(String address) {
        this.address = address;
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
    public void setDefaultCategory(String category) {
        if(category!=null) {
            defaultCategory = category;
        }
    }


    public void setMaxRepeat(int maxRepeat) {
        this.maxRepeat = maxRepeat;
    }
}