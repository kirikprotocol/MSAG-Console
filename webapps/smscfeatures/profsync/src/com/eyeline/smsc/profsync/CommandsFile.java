package com.eyeline.smsc.profsync;

import org.apache.log4j.Category;

import java.util.Properties;
import java.io.*;

/**
 * Created by Serge Lugovoy
 * Date: Dec 12, 2006
 * Time: 4:21:58 PM
 */
public class CommandsFile {
  static Category logger = Category.getInstance(CommandsFile.class);

  String fileName;
  File   file;
  File   rollFile;
  PrintWriter pw;
  BufferedReader rd;
  boolean haveRecords = false;

  public CommandsFile(Properties config) throws IOException {
    fileName = config.getProperty("backup.file");
    file = new File(fileName);
    rollFile = new File(fileName+".roll");
    pw = new PrintWriter(new FileWriter(file, true));
    if( file.length() > 0 ) haveRecords = true;
  }

  public synchronized boolean isHaveRecords() {
    return haveRecords;
  }

  public synchronized void roll() {
    if( pw != null ) {
      try {
        pw.close();
      } catch (Exception e) {
      }
      pw = null;
    }
    file.renameTo(rollFile);
    try {
      rd = new BufferedReader(new FileReader(rollFile));
      haveRecords = false;
    } catch (FileNotFoundException e) {
      rd = null;
      logger.error("Strange could not open rollfile", e);
      rollFile.renameTo(file);
    }
  }

  public synchronized String getCommand() {
    String res = null;
    if( rd != null ) {
      try {
        res = rd.readLine();
      } catch (IOException e) {
        logger.error("Could not read from roll file", e);
      }
      if( res == null ) {
        try {
          rd.close();
        } catch (IOException e) {
        }
        rd = null;
        rollFile.delete();
      }
    }
    return res;
  }

  public synchronized void addCommand(String cmd) throws IOException {
    if( pw == null ) {
      pw = new PrintWriter(new FileWriter(file, true));
    }
    pw.println(cmd);
    if( pw.checkError() ) throw new IOException();
    pw.flush();
    if( pw.checkError() ) throw new IOException();
    haveRecords = true;
  }
}
