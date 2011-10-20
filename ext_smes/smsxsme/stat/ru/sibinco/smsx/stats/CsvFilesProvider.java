package ru.sibinco.smsx.stats;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileFilter;
import java.io.FileReader;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

/**
 * User: artem
 * Date: 20.10.11
 */
public class CsvFilesProvider implements RecordsProvider {

  private final File csvDir;

  public CsvFilesProvider(File csvDir) {
    this.csvDir = csvDir;
  }

  public void processRecords(Date date, RecordHandler handler) throws Exception {
    SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyy-MMM" + File.separatorChar + "dd", Locale.ENGLISH);
    File dir = new File(csvDir, simpleDateFormat.format(date));

    FileFilter dirFilter = new FileFilter() {
      public boolean accept(File pathname) {
        return pathname.isDirectory();
      }
    };

    FileFilter fileFilter = new FileFilter() {
      public boolean accept(File pathname) {
        return pathname.isFile();
      }
    };

    if (dir.exists()) {
      File[] childDirectories = dir.listFiles(dirFilter);

      if (childDirectories != null) {
        for (File h : childDirectories) {
          System.out.println("Process dir: " + h.getParent() + File.separator + h.getName() + "... ");
          File[] csvFiles = h.listFiles(fileFilter);
          if (csvFiles != null) {
            for (File f : csvFiles)
              processFile(f, handler);
          }
          System.out.println("  OK.");
        }
      }
    }

  }

  private static void processFile(File f, RecordHandler handler) throws Exception {
    System.out.println("Process file: " + f.getName());
    BufferedReader reader = null;
    try {
      reader = new BufferedReader(new FileReader(f));
      String line = reader.readLine();
      if (line != null) {
        while ((line = reader.readLine()) != null) {
          if (line.length() == 0) {
            continue;
          }
          try {
            processLine(line, handler);
          } catch (Exception e) {
            System.out.println("Invalid file format: " + f.getAbsolutePath());
            System.out.println("Error line: " + line);
            e.printStackTrace();
          }
        }
      }
    } finally {
      if (reader != null) {
        reader.close();
      }
    }
  }

  private static void processLine(String line, RecordHandler handler) {
    //MSG_ID_0,RECORD_TYPE_1,MEDIA_TYPE_2,BEARER_TYPE_3,SUBMIT_4,FINALIZED_5,STATUS_6,SRC_ADDR_7,SRC_IMSI_8,SRC_MSC_9,SRC_SME_ID_10,DST_ADDR_11,DST_IMSI_12,DST_MSC_13,DST_SME_ID_14,DIVERTED_FOR_15,ROUTE_ID_16,SERVICE_ID_17,SERVICE_TYPE_18,USER_MSG_REF_19,DATA_LENGTH_20,PARTS_NUM_21,SMSX_SRV_22,MT_23,CONTRACT_24,CHARGE_25,IN_BILL_26

    String[] buffer = splitLine(line, 19);

    String route = buffer[16].trim();
    String sAddr = buffer[7];
    String dAddr = buffer[11];
    String srcSme = buffer[10];
    String srcMsc = buffer[9];
    int serviceId = Integer.parseInt(buffer[17]);

    handler.handleRecord(route, sAddr, dAddr, srcSme, srcMsc, serviceId);
  }

  private static String[] splitLine(String line, int limit) {
    String[] result = new String[limit];
    int oldpos = -0, pos;
    for (int i = 0; i < limit; i++) {
      pos = line.indexOf(',', oldpos);
      if (pos < 0)
        break;

      if (line.charAt(oldpos) == '"')
        result[i] = line.substring(oldpos + 1, pos - 1);
      else
        result[i] = line.substring(oldpos, pos);

      oldpos = pos + 1;
    }
    return result;
  }
}
