package ru.sibinco.smsx.stats.backend.datasource;

import ru.sibinco.smsx.stats.backend.StatisticsException;
import ru.sibinco.smsx.stats.backend.Visitor;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.regex.Pattern;

/**
 * @author Aleksandr Khalitov
 */
abstract class FileStatsProcessor {

  static final Pattern commaPattern = Pattern.compile(",");
  protected static char comma = ',';

  private final File artefactsDir;
  final Date from;
  final Date till;
  private final Progress p;

  FileStatsProcessor(File artefactsDir, Date from, Date till, Progress p) {
    this.artefactsDir = artefactsDir;
    this.from = from;
    this.till = till;
    this.p = p;
  }

  protected abstract LineVisitor getLineVisitor();
  protected abstract FilenameFilter getFilenameFilter();
  protected abstract void getResults(Visitor v) throws StatisticsException;



  protected final void process(Visitor v) throws StatisticsException {
    visitFiles(artefactsDir, getFilenameFilter(), getLineVisitor(), new Progress(){
      public void setProgress(int progress) {
        p.setProgress(3*progress/4);
      }
    });
    try{
      getResults(v);
    }finally {
      p.setProgress(100);
    }
  }


  final void visitFiles(File d, FilenameFilter filter, LineVisitor visitor, Progress p) throws StatisticsException {
    File[] fs = d.listFiles(filter);
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

  class StatsFileFilter implements FilenameFilter{
    private final String suffix;
    private final Date from;
    private final Date till;
    private final SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd");

    StatsFileFilter(String suffix, Date from, Date till) {
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
