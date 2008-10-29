package mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile;

import mobi.eyeline.smsquiz.replystats.Reply;

import java.util.Date;
import java.util.List;

public interface StatsFile {

  public void open() throws FileStatsException;

  public void add(Reply reply) throws FileStatsException;

  public List<Reply> getReplies(String oa, Date from, Date till) throws FileStatsException;

  public String getName();

  public void close();
}
 
