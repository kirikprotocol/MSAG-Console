package ru.sibinco.smsx.stats.backend;

import ru.sibinco.smsx.stats.backend.datasource.*;

import java.io.*;

/**
 * @author Aleksandr Khalitov
 */
class ResultsManager {

  private final File resultsDir;

  public ResultsManager(File resultsDir) {
    this.resultsDir = resultsDir;
  }

  SmsxUserResults createSmsxUsersResult(int requestId) throws StatisticsException{
    return new SmsxUserResults(requestId);
  }

  WebDailyResults createWebDailyResult(int requestId) throws StatisticsException{
    return new WebDailyResults(requestId);
  }

  TrafficResults createTrafficResult(int requestId) throws StatisticsException {
    return new TrafficResults(requestId);
  }

  WebRegionsResults createWebRegionsResult(int requestId) throws StatisticsException {
    return new WebRegionsResults(requestId);
  }

  SmsxResults getSmsxResult(final int reqId) {
    File dir = buildSmsxDir(reqId);
    if(!dir.exists()) {
      return null;
    }
    return new SmsxResults() {
      public void getSmsxUsers(Visitor visitor) throws StatisticsException {
        visit(buildSmsxUsersFile(reqId), visitor, new LineConverter() {
          public Object convert(String line) throws StatisticsException {
            return SmsxConverter.parseSmsxUsers(line); 
          }
        });
      }
      public void getWebDaily(Visitor visitor) throws StatisticsException { 
        visit(buildWebDailyFile(reqId), visitor, new LineConverter() {
          public Object convert(String line) throws StatisticsException {
            return SmsxConverter.parseWebDaily(line); 
          }
        });
      }
      public void getWebRegions(Visitor visitor) throws StatisticsException { 
        visit(buildWebRegionsFile(reqId), visitor, new LineConverter() {
          public Object convert(String line) throws StatisticsException {
            return SmsxConverter.parseWebRegion(line); 
          }
        });
      }
      public void getTraffic(Visitor visitor) throws StatisticsException {
        visit(buildTrafficFile(reqId), visitor, new LineConverter() {
          public Object convert(String line) throws StatisticsException {
            return SmsxConverter.parseTraffic(line); 
          }
        });
      }
    };
  }

  void removeSmsxResult(int reqId) throws StatisticsException{
    File dir = buildSmsxDir(reqId);
    if(dir.exists()) {
      recursiveDirRemove(dir);
    }
  }

  void removeSponsoredResult(int reqId) throws StatisticsException{
    File dir = buildSponsoredDir(reqId);
    if(dir.exists()) {
      recursiveDirRemove(dir);
    }
  }

  private static void recursiveDirRemove(File dir) throws StatisticsException{
    if(dir.isDirectory()) {
      File[] files = dir.listFiles();
      for(int i=0;i<files.length;i++) {
        File f = files[i];
        if(f.isDirectory()) {
          recursiveDirRemove(f);
        }else {
          if(!f.delete() && f.exists()) {
            throw new StatisticsException("Can't remove file: "+f.getAbsolutePath());
          }
        }
      }
      if(!dir.delete() && dir.exists()) {
        throw new StatisticsException("Can't remove dir: "+dir.getAbsolutePath());
      }
    }
  }
  
  private File buildSmsxDir(int requestId) {
    return new File(resultsDir, Integer.toString(requestId));
  }
  private File buildSponsoredDir(int requestId) {
    return new File(resultsDir, Integer.toString(requestId));
  }

  private File buildSmsxUsersFile(int requestId) {
    return new File(resultsDir, new StringBuffer().append(requestId).
        append(File.separatorChar).append("smsx-users.csv").toString());
  }

  private File buildWebDailyFile(int requestId) {  
    return new File(resultsDir, new StringBuffer().append(requestId).
        append(File.separatorChar).append("websms-daily.csv").toString());
  }

  private File buildTrafficFile(int requestId) { 
    return new File(resultsDir, new StringBuffer().append(requestId).
        append(File.separatorChar).append("traffic.csv").toString());
  }

  private File buildWebRegionsFile(int requestId) { 
    return new File(resultsDir, new StringBuffer().append(requestId).
        append(File.separatorChar).append("websms-regions.csv").toString());
  }


  class SmsxUserResults {
    private PrintWriter w;
    private SmsxUserResults(int requestId) throws StatisticsException {
      File f = buildSmsxUsersFile(requestId);
      File p = f.getParentFile();
      if(!p.exists() && !p.mkdirs()) {
        throw new StatisticsException("Can't create dir: "+p.getAbsolutePath());
      }       
      try{
        w = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(f),"windows-1251")));
        w.println("REGION,SERVICE_ID,COUNT");
      }catch (IOException e) {
        throw new StatisticsException(e);
      }
    }
    void write(SmsxUsers users) throws StatisticsException{
      SmsxConverter.write(w, users);
    }
    void close() {
      if(w != null) {
        w.close();
      }
    }
  }

  class WebDailyResults {
    private PrintWriter w;
    private WebDailyResults(int requestId) throws StatisticsException {
      File f = buildWebDailyFile(requestId);
      File p = f.getParentFile();
      if(!p.exists() && !p.mkdirs()) {
        throw new StatisticsException("Can't create dir: "+p.getAbsolutePath());
      }       
      try{
        w = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(f),"windows-1251")));
        w.println("DATE,REGION,MSC,COUNT");
      }catch (IOException e) {
        throw new StatisticsException(e);
      }
    }
    void write(WebDaily webDaily) throws StatisticsException{
      SmsxConverter.write(w, webDaily);
    }
    void close() {
      if(w != null) {
        w.close();
      }
    }
  }

  class TrafficResults {
    private PrintWriter w;
    private TrafficResults(int requestId) throws StatisticsException {  
      File f = buildTrafficFile(requestId);
      File p = f.getParentFile();
      if(!p.exists() && !p.mkdirs()) {
        throw new StatisticsException("Can't create dir: "+p.getAbsolutePath());
      } 
      try{
        w = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(f),"windows-1251")));
        w.println("REGION,SERVICE_ID,MSC,COUNT");
      }catch (IOException e) {
        throw new StatisticsException(e);
      }
    }
    void write(Traffic traffic) throws StatisticsException {
      SmsxConverter.write(w, traffic);
    }
    void close() {
      if(w != null) {
        w.close();
      }
    }
  }

  class WebRegionsResults {
    private PrintWriter w;
    private WebRegionsResults(int requestId) throws StatisticsException { 
      File f = buildWebRegionsFile(requestId);
      File p = f.getParentFile();
      if(!p.exists() && !p.mkdirs()) {
        throw new StatisticsException("Can't create dir: "+p.getAbsolutePath());
      } 
      try{
        w = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(f),"windows-1251")));
        w.println("REGION,MSC,SRC_COUNT,DST_COUNT");
      }catch (IOException e) {
        throw new StatisticsException(e);
      }
    }
    void write(WebRegion webRegion) throws StatisticsException {
      SmsxConverter.write(w, webRegion);
    }
    void close() {
      if(w != null) {
        w.close();
      }
    }
  }
   

  private void visit(File file, Visitor visitor, LineConverter converter) throws StatisticsException {
    if(!file.exists()) {
     return;
    }
    BufferedReader reader = null;
    try{
      reader = new BufferedReader(new InputStreamReader(new FileInputStream(file), "windows-1251"));
      String line = reader.readLine();
      if(line != null) {
        while((line = reader.readLine()) != null) {
          if(!visitor.visit(converter.convert(line))) {
            return;
          }
        }
      }
    } catch (IOException e){
      throw new StatisticsException(e);
    } finally {
      if(reader != null) {
        try{
          reader.close();
        }catch (IOException ignored) {}
      }
    }

  }


  private static interface LineConverter{
    Object convert(String line) throws StatisticsException;
  }

}
