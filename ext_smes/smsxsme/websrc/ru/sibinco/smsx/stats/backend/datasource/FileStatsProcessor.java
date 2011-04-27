package ru.sibinco.smsx.stats.backend.datasource;

import ru.sibinco.smsx.stats.backend.StatisticsException;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
class FileStatsProcessor {

  private final File artefactsDir;
  final Date from;
  final Date till;

  FileStatsProcessor(File artefactsDir, Date from, Date till) {
    this.artefactsDir = artefactsDir;
    this.from = from;
    this.till = till;
  }
//
//  protected abstract LineVisitor getLineVisitor();
//  protected abstract FilenameFilter getFilenameFilter();
//  protected abstract void getResults(Visitor v) throws StatisticsException;




  final void visitFiles(FilenameFilter filter, LineVisitor visitor, ProgressListener p) throws StatisticsException {
    File[] fs = artefactsDir.listFiles(filter);
    int count = 0;
    for(int i=0; i<fs.length;i++) {
      File f = fs[i];
      count++;
      BufferedReader reader = null;
      try{
        reader = new BufferedReader(new InputStreamReader(new FileInputStream(f),"windows-1251"));
        String line = reader.readLine();
        if(line != null) {
          while((line = reader.readLine()) != null) {
            if(line.length() == 0) {
              continue;
            }
            visitor.visit(f.getName(), line);
          }
        }
        p.setProgress(100*count/fs.length);
      }catch (IOException e){
        throw new StatisticsException(e);
      }finally {
        if(reader != null) {
          try{
          reader.close();
          }catch (IOException ignored){}
        }
      }
    }
  }


  protected interface LineVisitor {
    void visit(String fileName, String line) throws StatisticsException;
  }


  public static class StatsFileFilter implements FilenameFilter{
    private final String suffix;
    private final Date from;
    private final Date till;
    private final SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd");

    public StatsFileFilter(String suffix, Date from, Date till) {
      this.suffix = suffix;
      this.from = from;
      this.till = till;
    }

    public boolean accept(File dir, String name) {
      if(!name.endsWith(suffix)) {
        return false;
      }
      if(from == null && till == null) {
        return true;
      }
      try {
        name = name.substring(0, name.indexOf('-'));
        Date date = sdf.parse(name);
        return !(from != null && from.compareTo(date) > 0 || till != null && till.compareTo(date) < 0);
      } catch (ParseException e) {
        return false;
      }
    }
  }
}
