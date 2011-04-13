package ru.sibinco.sponsored.stats.backend;

import ru.sibinco.sponsored.stats.backend.datasource.SponsoredConverter;

import java.io.*;

/**
 * @author Aleksandr Khalitov
 */
class ResultsManager {

  private final File resultsDir;

  public ResultsManager(File resultsDir) {
    this.resultsDir = resultsDir;
  }
  
  SponsoredResults createSponsoredResult(int requestid) throws StatisticsException {
    return new SponsoredResults(requestid);
  }

  void getSponsoredResults(int reqId, Visitor visitor) throws StatisticsException{ 
    File file = buildSponsoredFile(reqId);
    if(!file.exists()) {
      return;
    }
    visit(file, visitor, new LineConverter() {
      public Object convert(String line) throws StatisticsException {
        return SponsoredConverter.parseRecord(line);
      }
    });
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

  private File buildSponsoredDir(int requestId) {
    return new File(resultsDir, Integer.toString(requestId));
  }
  
  private File buildSponsoredFile(int requestId) { 
    return new File(resultsDir, new StringBuffer().append(requestId).
        append(File.separatorChar).append("sponsored.csv").toString());
  }
  
  class SponsoredResults { 
    private PrintWriter w;
    private SponsoredResults(int requestId) throws StatisticsException {     
      File f = buildSponsoredFile(requestId);
      File p = f.getParentFile();
      if(!p.exists() && !p.mkdirs()) {
        throw new StatisticsException("Can't create dir: "+p.getAbsolutePath());
      } 
      try{
        w = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(f),"windows-1251")));
        w.println("ADDRESS,BONUS");
      }catch (IOException e) {
        throw new StatisticsException(e);
      }
    }
    void write(String address, float bonus) throws StatisticsException{
      SponsoredConverter.write(w, address, bonus);
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
