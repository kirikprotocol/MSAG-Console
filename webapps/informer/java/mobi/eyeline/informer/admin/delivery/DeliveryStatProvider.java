package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.util.CSVTokenizer;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Интерфейс, обеспечивающий досуп к статистике
 * @author Artem Snopkov
 */
public class DeliveryStatProvider {
  File baseDir;
  FileSystem fileSys;
  String subDirNameFormat;

  public DeliveryStatProvider(File directory,FileSystem fileSys) {
    this(directory,fileSys,"yyyyMMdd");
  }

  DeliveryStatProvider(File directory,FileSystem fileSys,String subDirNamePattern) {
    baseDir = directory;
    this.fileSys = fileSys;
    this.subDirNameFormat= subDirNamePattern;
  }

  /**
   * Поочередно передает в visitor все записи статистики, удовлетворяющие условиям, накладываемыми в filter.
   * Процесс продолжается до тех пор, пока метод visit в visitor возвращает true, либо записи не закончатся.
   * Если filter == null, то провайдер перебирает все записи.
   * @param filter фильтр, описывающий ограничения на записи
   * @param visitor визитор, обрабатывающий найденные записи
   * @throws AdminException если произошла ошибка при обращении к стораджу статистики
   */
  public void accept(DeliveryStatFilter filter, DeliveryStatVisitor visitor) throws AdminException {
    try {
      SimpleDateFormat fmt = new SimpleDateFormat(subDirNameFormat);
      String minSubDirName = null;
      String maxSubDirName = null;
      Calendar cFrom = null;
      Calendar cTo=null;
      if(filter!=null) {
        if(filter.getFromDate()!=null) {
          cFrom  = Calendar.getInstance();
          cFrom.setTime(filter.getFromDate());
          minSubDirName = fmt.format(cFrom.getTime());
        }
        if(filter.getTillDate()!=null) {
          cTo  = Calendar.getInstance();
          cTo.setTime(filter.getTillDate());
          maxSubDirName = fmt.format(cTo.getTime());
        }
      }

      String[] subDirs = fileSys.list(baseDir);
      Arrays.sort(subDirs);
      List<SubDirProcessor> subDirProcessors  = new ArrayList<SubDirProcessor>();
      int totalFilesCount=0;
      for(String subDirName : subDirs) {
        int fromHour=0;
        int toHour=23;
        int fromMin=0;
        int toMin=59;
        if(minSubDirName!=null) {
          int compareMin = subDirName.compareTo(minSubDirName);
          if(compareMin<0) continue;
          else if(compareMin==0) {
            fromHour = cFrom.get(Calendar.HOUR_OF_DAY);
            fromMin  = cFrom.get(Calendar.MINUTE);
          }
        }
        if(maxSubDirName!=null) {
          int compareMax = subDirName.compareTo(maxSubDirName);
          if(compareMax>0) break;
          else if(compareMax==0) {
            toHour   = cTo.get(Calendar.HOUR_OF_DAY);
            toMin    = cTo.get(Calendar.MINUTE);
          }
        }
        SubDirProcessor sdProcessor = new SubDirProcessor(subDirName,fmt,fromHour,fromMin,toHour,toMin);
        subDirProcessors.add(sdProcessor);
        totalFilesCount+=sdProcessor.getFilesCount();
      }

      int currentFile=0;
      for(SubDirProcessor p : subDirProcessors) {
        p.process(filter,visitor,currentFile,totalFilesCount);
        currentFile+=p.getFilesCount();
      }
    }
    catch (IOException e) {
      throw new DeliveryStatException("filesys.ioexception",e);
    }
  }


  private String formatInt(int val, int positions) {
    StringBuilder sb = new StringBuilder(Integer.toString(val));
    while(sb.length()<positions) sb.insert(0,'0');
    return sb.toString();
  }


  class SubDirProcessor {
    String subDirName;
    File subDir;
    int year;
    int month;
    int day;
    List<FileProcessor> fileProcessors;
    int filesCount;

    public SubDirProcessor(String subdirName,SimpleDateFormat fmt, int fromHour,int fromMin,int toHour,int toMin) throws AdminException {
      try {
        this.subDirName = subdirName;
        subDir = new File(baseDir,subdirName);
        filesCount=0;
        fileProcessors = new ArrayList<FileProcessor>();
        Calendar c = Calendar.getInstance();
        c.clear();
        c.setTime(fmt.parse(subDirName));
        year  = c.get(Calendar.YEAR);
        month = c.get(Calendar.MONTH)+1;
        day   = c.get(Calendar.DATE);
        for(int hour = fromHour; hour<=toHour; hour++) {
          String fName = formatInt(hour,2)+".csv";
          File f = new File(subDir,fName);
          if(fileSys.exists(f)) {
            fileProcessors.add(new FileProcessor(f,year,month,day,hour,hour==fromHour ? fromMin:0, hour==toHour ? toMin : 59));
            filesCount++;
          }
        }
      }
      catch (ParseException e) {
        throw new DeliveryStatException("error.parsing.subdir.name",e,subDirName);
      }
    }

    public int getFilesCount() {
      return filesCount;
    }

    public void process(DeliveryStatFilter filter, DeliveryStatVisitor visitor, int currentFile, int totalFilesCount) throws AdminException, IOException {
      for(FileProcessor fp : fileProcessors) {
        if(!fp.process(filter,visitor,currentFile,totalFilesCount)) break;
        currentFile++;
      }
    }
  }

  class FileProcessor {
    File file;
    int year;
    int month;
    int day;
    int hour;
    int fromMin;
    int toMin;

    FileProcessor(File file, int year, int month, int day, int hour, int fromMin, int toMin) {
      this.file=file;
      this.year = year;
      this.month = month;
      this.day = day;
      this.hour = hour;
      this.fromMin = fromMin;
      this.toMin = toMin;
    }

    public boolean process(DeliveryStatFilter filter, DeliveryStatVisitor visitor, int currentFile, int totalFilesCount) throws AdminException, IOException {
      InputStream is=null;
      try {
        is = fileSys.getInputStream(file);
        BufferedReader reader = new BufferedReader(new InputStreamReader(is));

        //skip first line
        if(reader.readLine()==null) return true;

        String line;
        while(( line = reader.readLine())!=null) {
          CSVTokenizer tokenizer = new CSVTokenizer(line);
          //TASK_ID,USER,MINUTE,DELIVERED,FAILED
          if(tokenizer.hasMoreTokens()) {

            int taskId = Integer.parseInt(tokenizer.nextToken());
            if(filter!=null && filter.getTaskId()!=null && filter.getTaskId()!=taskId) continue;

            String user = tokenizer.nextToken();
            if(filter!=null && filter.getUser()!=null && !filter.getUser().equals(user) ) continue;

            int minute = Integer.parseInt(tokenizer.nextToken());
            if(minute < fromMin) continue;
            if(minute > toMin)   break;

            int delivered = Integer.parseInt(tokenizer.nextToken());
            int failed = Integer.parseInt(tokenizer.nextToken());

            DeliveryStatRecord rec = new DeliveryStatRecord(user, year,month,day,hour,minute,taskId,delivered,failed);
            if(!visitor.visit(rec,totalFilesCount,currentFile)) {
              return false;
            }
          }
        }
      }
      finally {
        if(is!=null) try {
          is.close();
        }
        catch (IOException e) {}
      }
      return true;

    }
  }

}
