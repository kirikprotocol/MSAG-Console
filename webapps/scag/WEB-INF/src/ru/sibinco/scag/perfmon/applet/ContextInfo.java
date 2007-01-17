package ru.sibinco.scag.perfmon.applet;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 15.01.2007
 * Time: 18:57:19
 * To change this template use File | Settings | File Templates.
 */
public class ContextInfo {
    private String name;
    private int pixPerSecond;
    private int scale;
    private int block;
    private ContextInfo http;
    private ContextInfo smpp;

    public ContextInfo() {
      http = new ContextInfo("HTTP");
      smpp = new ContextInfo("SMPP");
    }

    public ContextInfo(String name) {
      this.name = name;
    }

    public void initHttp() {
      http.store();
    }

    public void initSmpp() {
      smpp.store();
    }

    public void setContextInfo(String curStatMode, String prevStatMode) {
      ContextInfo curInfo = getByName(curStatMode);
      ContextInfo prevInfo = getByName(prevStatMode);
      prevInfo.store();
      curInfo.set();
    }

    private ContextInfo getByName(String name) {
      if (name.equals(PerfMon.smppStatMode))
        return smpp;
      else return http;
    }

    private void store() {
      this.pixPerSecond = PerfMon.pixPerSecond;
      this.scale = PerfMon.scale;
      this.block = PerfMon.block;
      //System.out.println("STORED : " + this);
    }

    private void set() {
      PerfMon.pixPerSecond = this.pixPerSecond;
      PerfMon.scale = this.scale;
      PerfMon.block = this.block;
      //System.out.println("SETTED : " + this);
    }

    public String toString() {
      return this.name +" : "+
             "pixPerSecond = " +this.pixPerSecond + " : " +
             "scale = " + this.scale + " : " +
             "block = " + this.block;
    }
}
