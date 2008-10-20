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

  public FileReplyStatsDataSource(final String config) throws ReplyDataSourceException {
    try {
      filesCache = new StatsFilesCache(config);
    } catch (FileStatsException e) {
      throw new ReplyDataSourceException("Error init files cache", e);
    }
  }

  public void add(Reply reply) throws ReplyDataSourceException {
    String da = reply.getDa();
    Date date = reply.getDate();
    if ((da == null) || (date == null)) {
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
      if (file != null) {
        file.close();
      }
    }

  }


 
  public Reply getLastReply(String oa, String da, Date from, Date till) throws ReplyDataSourceException {
    Reply reply = null;
    if ((oa == null) || (da == null) || (from == null) || (till == null)) {
      logger.error("Some arguments are null");
      throw new ReplyDataSourceException("Some arguments are null", ReplyDataSourceException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    Collection<StatsFile> files;
    try {
      files = filesCache.getFiles(da, from, till);
    } catch (FileStatsException e) {
      logger.error("Error during getting file list", e);
      throw new ReplyDataSourceException("Error during getting file list", e);
    }
    List<Reply> replies;
    for (StatsFile file : files) {
      try {
        file.open();
        replies = file.getReplies(oa, from, till);
        if (replies == null) {
          continue;
        }
        for (Reply aR : replies) {
          if ((reply == null) || (reply.getDate().compareTo(aR.getDate()) <= 0)) {
            reply = aR;
          }
        }
      } catch (FileStatsException e) {
        logger.error("Error during getting replies", e);
        throw new ReplyDataSourceException("Error during getting replies", e);
      } finally {
        if (file != null) {
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
 
