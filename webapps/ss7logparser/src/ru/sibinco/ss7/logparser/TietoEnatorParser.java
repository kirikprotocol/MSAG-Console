package ru.sibinco.ss7.logparser;

import java.text.*;
import java.util.*;
import java.io.*;

/**
 * Created by: Serge Lugovoy
 * Date: 18.10.2004
 * Time: 10:17:51
 */
public class TietoEnatorParser {
  public static void main(String args[]) {
    TietoEnatorParser lll = new TietoEnatorParser();
    lll.dojob(args);
  }

  public void dojob(String args[]) {
    try {
      String inFile = null;
      String outFile = null;
      long fromdate = -1;
      long todate = -1;
      boolean receivedOnly = false;
      HashSet typeFilter = null;
      SimpleDateFormat argdf = new SimpleDateFormat("yyyy-MM-dd  HH:mm:ss", Locale.ENGLISH);
      try {
        for( int i = 0; i < args.length; i++ ) {
          if( args[i].equals("-in") ) {
            inFile = args[++i];
          } else if( args[i].equals("-out") ) {
            outFile = args[++i];
          } else if( args[i].equals("-from") ) {
            fromdate = argdf.parse(args[++i]).getTime();
          } else if( args[i].equals("-to") ) {
            todate = argdf.parse(args[++i]).getTime();
          } else if( args[i].equals("-ro") ) {
            receivedOnly = true;
          } else if( args[i].equals("-types") ) {
            typeFilter = new HashSet();
            StringTokenizer stk = new StringTokenizer(args[++i], ",");
            while( stk.hasMoreTokens() ) {
              String tk = stk.nextToken().trim();
              int id = LogBlock.lookupType(tk);
              if( id == 0 ) throw new ParseException("Invalid type specified: "+tk, 0);
              typeFilter.add( new Integer(id) );
            }
          }
        }
      } catch (Exception e) {
        e.printStackTrace();
        System.out.println(
            "Usage: TietoEnatorParser -in infile -out outfile"+
            "[-from yyyy-MM-dd  HH:mm:ss] [-to yyyy-MM-dd  HH:mm:ss]"+
            "[-ro] [-types TCAP,MAP,SCCP...]"
        );
      }

      File f = new File(inFile);
      BufferedReader rd = new BufferedReader(new FileReader(f));
      ArrayList blocks = new ArrayList();
      ArrayList lines = new ArrayList();
      LogBlock logBlock = null;
      String line = null;
      boolean newblock = true;
      boolean skipblock = false;
      int tcount = 0, fcount = 0, lcount = 0;
      System.out.println("Reading packets from: "+inFile);
      while( (line =rd.readLine()) != null ) {
        lcount++;
        line = line.trim();
        if(line.length() == 0 ) {
          newblock = true;
          skipblock = true;
          continue;
        }
        if(newblock) {
          if( line.startsWith("RECEIVED:") || line.startsWith("SENT:") ) {
            // new packet
            if( lines.size() > 0 ) {
              logBlock = LogBlock.createInstance(lines);
              tcount++;
              boolean accepted = true;
              if( receivedOnly && logBlock.sent ) accepted = false;
              if( accepted && fromdate != -1 && logBlock.time < fromdate ) accepted = false;
              if( accepted && todate != -1 && logBlock.time < todate ) accepted = false;
              if( accepted && typeFilter != null && !typeFilter.contains(new Integer(logBlock.type) ) ) accepted = false;
              if( accepted ) {
                fcount++;
                blocks.add(logBlock);
              }
              if( (tcount%100) == 0 ) {
                System.out.print("Packets: "+fcount+"/"+tcount+" lines: "+lcount+"\r");
              }
            }
            lines.clear();
            skipblock = false;
          } else {
            // should skip unnecessary info blocks
            skipblock = true;
          }
          newblock = false;
        }
        if( !skipblock || line.startsWith("****") ) {
          lines.add(line);
        }
      }
      if( lines.size() > 0 ) {
        logBlock = LogBlock.createInstance(lines);
        tcount++;
        boolean accepted = true;
        if( receivedOnly && logBlock.sent ) accepted = false;
        if( accepted && fromdate != -1 && logBlock.time < fromdate ) accepted = false;
        if( accepted && todate != -1 && logBlock.time < todate ) accepted = false;
        if( accepted && typeFilter != null && !typeFilter.contains(new Integer(logBlock.type)) ) accepted = false;
        if( accepted ) {
          fcount++;
          blocks.add(logBlock);
        }
        if( (tcount%100) == 0 ) {
          System.out.print("Packets: "+fcount+"/"+tcount+" lines: "+lcount+"\r");
        }
      }
      rd.close();
      System.out.println("Total packets read: "+fcount+"/"+tcount+" lines: "+lcount);
      System.out.println("Sorting results");
      Collections.sort(blocks);
      System.out.println("Writing results");
      PrintWriter pw = new PrintWriter( new FileWriter(outFile) );
      int pcount = 0;
      for(Iterator it = blocks.iterator(); it.hasNext(); ) {
        LogBlock l = (LogBlock) it.next();
        l.write(pw);
        pw.println();
        pcount++;
        if( (pcount%100) == 0 ) {
          System.out.print(pcount+" of "+blocks.size()+" written\r");
        }
      }
      pw.close();
      System.out.println(pcount+" of "+blocks.size()+" total written. The end.");

    } catch (Exception e) {
      e.printStackTrace();
    }
  }

}
