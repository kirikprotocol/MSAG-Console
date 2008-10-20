package mobi.eyeline.smsquiz.replystats.datasource.impl;

import mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile.StatsFilesCache;
import mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile.StatsFile;
import mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile.FileStatsException;
import mobi.eyeline.smsquiz.replystats.Reply;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyStatsDataSource;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyDataSourceException;

import java.util.*;
import java.io.File;

import org.apache.log4j.Logger;

public class FileReplyStatsDataSource implements ReplyStatsDataSource {

	private static final Logger logger = Logger.getLogger(FileReplyStatsDataSource.class);
	private StatsFilesCache filesCache;

    public FileReplyStatsDataSource(final String config) throws ReplyDataSourceException{
        try {
            filesCache = new StatsFilesCache(config);
        } catch (FileStatsException e) {
            throw new ReplyDataSourceException("Error init files cache",e);
        }
    }

    public void add(Reply reply) throws ReplyDataSourceException {
        String da = reply.getDa();
        Date date = reply.getDate();
        if((da==null)||(date==null)) {
            logger.error("Some arguments are null");
            throw new ReplyDataSourceException("Some arguments are null", ReplyDataSourceException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        StatsFile file = null;
        try {
            file = filesCache.getFile(reply.getDa(), reply.getDate());
            file.open();
            file.add(reply);
        } catch (FileStatsException e) {
            logger.error("Error during write the reply");
            throw new ReplyDataSourceException("Error during write the reply", e);
        } finally {
            if(file!=null) {
                file.close();
            }
        }

    }
	 

	public Collection<Reply> list(String da, Date from, Date till) throws ReplyDataSourceException{
        if((da == null)||(from == null)||(till == null)) {
            logger.error("Some arguments are null");
            throw new ReplyDataSourceException("Some arguments are null", ReplyDataSourceException.ErrorCode.ERROR_WRONG_REQUEST);
        }

        Collection<Reply> replies = new LinkedList<Reply>();
        Collection<StatsFile> files;

        try {
            files = filesCache.getFiles(da,from,till);
        } catch (FileStatsException e) {
            logger.error("Error during getting file list",e);
            throw new ReplyDataSourceException("Error during getting file list",e);
        }

        for(StatsFile file : files) {
            try {
                file.open();
                file.list(from, till,replies);
            } catch (FileStatsException e) {
                logger.error("Error during getting replies",e);
                throw new ReplyDataSourceException("Error during getting replies",e);
            } finally {
                if(file!=null) {
                    file.close();
                }
            }
        }
        return replies;
    }

    public Map<String,Reply> listMap(String da, Date from, Date till) throws ReplyDataSourceException{
        if((da == null)||(from == null)||(till == null)) {
            logger.error("Some arguments are null");
            throw new ReplyDataSourceException("Some arguments are null", ReplyDataSourceException.ErrorCode.ERROR_WRONG_REQUEST);
        }

        HashMap<String,Reply> replies = new HashMap<String,Reply>();
        Collection<StatsFile> files;
        Collection<Reply> repliesInFile = new LinkedList<Reply>();

        try {
            files = filesCache.getFiles(da,from,till);
        } catch (FileStatsException e) {
            logger.error("Error during getting file list",e);
            throw new ReplyDataSourceException("Error during getting file list",e);
        }

        for(StatsFile file : files) {
            try {
                repliesInFile.clear();
                file.open();
                file.list(from, till,repliesInFile);
                for(Reply reply:repliesInFile) {
                    replies.put(reply.getOa(),reply);
                }
            } catch (FileStatsException e) {
                logger.error("Error during getting replies",e);
                throw new ReplyDataSourceException("Error during getting replies",e);
            } finally {
                if(file!=null) {
                    file.close();
                }
            }
        }
        return replies;
    }

    public Reply getLastReply(String oa, String da, Date from, Date till) throws ReplyDataSourceException{
        Reply reply = null;
        if((oa==null)||(da == null)||(from == null)||(till == null)) {
            logger.error("Some arguments are null");
            throw new ReplyDataSourceException("Some arguments are null", ReplyDataSourceException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        Collection<StatsFile> files;
        try {
            files = filesCache.getFiles(da,from,till);
        } catch (FileStatsException e) {
            logger.error("Error during getting file list",e);
            throw new ReplyDataSourceException("Error during getting file list",e);
        }
        StringTokenizer tokenizer;
        String position;
        List<Reply> replies;
        for(StatsFile file : files) {
            try {
                file.open();
                replies = file.getReplies(oa,from, till);
                if(replies==null) {
                    continue;
                }
                for(Reply aR: replies) {
                    if((reply==null)||(reply.getDate().compareTo(aR.getDate())<=0)) {
                        reply = aR;
                    }
                }
            } catch (FileStatsException e) {
                logger.error("Error during getting replies",e);
                throw new ReplyDataSourceException("Error during getting replies",e);
            } finally {
                if(file!=null) {
                    file.close();
                }
            }
        }
        return reply;
    }

    public void shutdown() {
	    filesCache.shutdown();
	}
	 
}
 
