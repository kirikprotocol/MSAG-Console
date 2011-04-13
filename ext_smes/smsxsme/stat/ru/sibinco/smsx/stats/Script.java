package ru.sibinco.smsx.stats;

import com.eyeline.utils.tree.radix.TemplatesRTree;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.regex.Pattern;

/**
 * @author Aleksandr Khalitov
 */
public class Script {

   //   "./ 20110101 conf output"
  public static void main(String[] args) throws Exception {
//    Thread.sleep(10000);
    long time = System.currentTimeMillis();
    Script s = new Script();
    s.init(args);
    s.process();
    System.out.println("Time: " + (System.currentTimeMillis() - time));
    System.out.println("Done!");
//    Thread.sleep(20000);
    System.out.println("Count:"+s.count.size());
    System.out.println("Src:"+s.dstAddrMap.size());
    System.out.println("Dst:"+s.srcAddrMap.size());
  }


  private Date date;

  private File outputDir;

  private File csvDir;

  private Set<String> routes;

  private TemplatesRTree<String> regions;

  void init(String[] args) throws Exception{
    if(args.length<4) {
      throw new IllegalArgumentException("Use with argument: <csv_dir> <yyyyMMdd> <routes_dir> <output_dir>");
    }

    csvDir = new File(args[0]);
    if(!csvDir.exists()) {
      throw new IllegalArgumentException("Directory with csv files doesn't exist: "+csvDir.getAbsolutePath());
    }
    date = new SimpleDateFormat("yyyyMMdd").parse(args[1]);
    File routesXml = new File(args[2], "routes.xml");
    File routesCsv = new File(args[2], "routes.csv");
    if(!routesCsv.exists()) {
      throw new IllegalArgumentException("File doesn't exist: "+ routesCsv.getAbsolutePath());
    }
    if(!routesXml.exists()) {
      throw new IllegalArgumentException("File doesn't exist: "+ routesXml.getAbsolutePath());
    }
    outputDir = new File(args[3]);
    if(!outputDir.exists() && !outputDir.mkdirs()) {
      System.out.println("Can't create output directory: "+outputDir.getAbsolutePath());
    }

    routes = Routes.parseRoutes(routesCsv);
    regions = buildTree(routesXml);
  }

  void process() throws Exception{

    File dir = new File(csvDir, new SimpleDateFormat("yyyy-MMM"+File.separatorChar+"dd", Locale.ENGLISH).format(date));

    if(dir.exists()) {
    String[] buffer = new String[19];
      for(File h : dir.listFiles(new FileFilter() {
        public boolean accept(File pathname) {
          return pathname.isDirectory();
        }
      })) {
        for(File f : h.listFiles(new FileFilter() {
          public boolean accept(File pathname) {
            return pathname.isFile();
          }
        })) {
          processFile(f, buffer);
        }
      }
    }
    publishResults();
  }

  private static final String unknownRegion = "Unknown";

  private String getRegionByAddress(String address) {
    String res = regions.get(address);
    return res != null ? res : unknownRegion;
  }

  private static void writeSmsx(PrintWriter smsxWriter, int serviceId, String address, String region) {
    smsxWriter.print(serviceId);
    smsxWriter.print(',');
    smsxWriter.print(address);
    smsxWriter.print(',');
    smsxWriter.println(region);
  }

  private static void writeWebSms(PrintWriter websmsWriter, int index, boolean isMsc, String address, String region) {
    websmsWriter.print(index);
    websmsWriter.print(isMsc ? ",1," : ",0,");
    websmsWriter.print(address);
    websmsWriter.print(',');
    websmsWriter.println(region);
  }

  private void  writeSrcDstStats() throws IOException{
    PrintWriter websmsWriter = null;
    try{
      websmsWriter = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(new File(outputDir, new SimpleDateFormat("yyyyMMdd").format(date)+"-websms-users.csv")), "windows-1251")));
      websmsWriter.println("INDEX,MSC,ADDRESS,REGION");
      PrintWriter smsxWriter = null;
      try{
        smsxWriter = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(new File(outputDir, new SimpleDateFormat("yyyyMMdd").format(date)+"-smsx-users.csv")), "windows-1251")));
        smsxWriter.println("SERVICE_ID,SRC_ADDRESS,REGION");
        for(Map.Entry<String, SrcAddr> s : srcAddrMap.entrySet()) {
          String region = getRegionByAddress(s.getKey());
          for(Integer serviceId : s.getValue().smsxServices) {
            writeSmsx(smsxWriter, serviceId, s.getKey(), region);
          }
          if(s.getValue().isWebSmsMsc != null) {
            writeWebSms(websmsWriter, 1, s.getValue().isWebSmsMsc, s.getKey(), region);
          }
        }
      }finally {
        if(smsxWriter != null) {
          smsxWriter.close();
        }
      }
      for(Map.Entry<String, Boolean> s : dstAddrMap.entrySet()) {
        String region = getRegionByAddress(s.getKey());
        writeWebSms(websmsWriter, 2, s.getValue(), s.getKey(), region);
      }
    }finally {
      if(websmsWriter != null) {
        websmsWriter.close();
      }
    }
  }

  private void writeCountStats() throws IOException {
    PrintWriter countWriter = null;
    try{
      countWriter = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(new File(outputDir, new SimpleDateFormat("yyyyMMdd").format(date)+"-traffic.csv")), "windows-1251")));
      countWriter.println("SERVICE_ID,MSC,REGION,SRC_SME_ID,COUNT");
      for(Map.Entry<CountKey, Integer> e : count.entrySet()) {
        countWriter.print(e.getKey().serviceId);
        countWriter.print(e.getKey().isMsc ? ",1," : ",0,");
        countWriter.print(e.getKey().region);
        countWriter.print(',');
        countWriter.print(e.getKey().srcSmeId);
        countWriter.print(',');
        countWriter.println(e.getValue());
      }
    }finally {
      if(countWriter != null) {
        countWriter.close();
      }
    }
  }

  private void publishResults() throws IOException {
    writeSrcDstStats();
    writeCountStats();
  }

  private TemplatesRTree<String> buildTree(File routesXml) throws Exception {
    Collection<Routes.Region> routes = Routes.parseRegions(routesXml);
    TemplatesRTree<String> regions = new TemplatesRTree<String>();
    for (Routes.Region r : routes) {
      for(String m : r.getMasks()) {
        regions.put(m, r.getId());
      }
    }
    return regions;
  }

  private void processFile(File f, String[] buffer) throws Exception {
    BufferedReader reader = null;
    try{
      reader = new BufferedReader(new FileReader(f));
      String line = reader.readLine();
      if(line != null) {
        while((line = reader.readLine()) != null){
          if(line.isEmpty()) {
            continue;
          }
          try{
            processLine(line, buffer);
          }catch (Exception e){
            System.out.println("Error line: "+line);
            e.printStackTrace();
          }
        }
      }
    }finally {
      if(reader != null) {
        reader.close();
      }
    }
  }



  private Map<String, SrcAddr> srcAddrMap = new HashMap<String, SrcAddr>(100000);
  private Map<String, Boolean> dstAddrMap = new HashMap<String, Boolean>(100000);
  private Map<CountKey, Integer> count = new HashMap<CountKey, Integer>(1000);

  private static Pattern p = Pattern.compile(",");

  private static final char quote = '\"';

  private String removeQuotes(String s) {
    if(s.length()>0 && s.charAt(0) == quote) {
      return s.substring(1,s.length()-1);
    }
    return s;
  }

  private static final String websms = "websms";
  private static final String websyssms = "websyssms";

  private static boolean isWebSms(String srcSmeId) {
    return srcSmeId.equals(websms) || srcSmeId.equals(websyssms);
  }

  private void processLine(String line, String[] buffer) throws InterruptedException {
    //MSG_ID_0,RECORD_TYPE_1,MEDIA_TYPE_2,BEARER_TYPE_3,SUBMIT_4,FINALIZED_5,STATUS_6,SRC_ADDR_7,SRC_IMSI_8,SRC_MSC_9,SRC_SME_ID_10,DST_ADDR_11,DST_IMSI_12,DST_MSC_13,DST_SME_ID_14,DIVERTED_FOR_15,ROUTE_ID_16,SERVICE_ID_17,SERVICE_TYPE_18,USER_MSG_REF_19,DATA_LENGTH_20,PARTS_NUM_21,SMSX_SRV_22,MT_23,CONTRACT_24,CHARGE_25,IN_BILL_26

    buffer = p.split(line, 19);

    String route = removeQuotes(buffer[16]).trim();
    if(!routes.contains(route)) {
      System.out.println("Route is not allowed: "+route);
      return;
    }

    String sAddr = removeQuotes(buffer[7]);
    String dAddr = removeQuotes(buffer[11]);
    String srcSme = removeQuotes(buffer[10]).trim();
    boolean srcMsc = !removeQuotes(buffer[9]).trim().isEmpty();
    int serviceId = Integer.parseInt(buffer[17]);

    String dstRegion = getRegionByAddress(dAddr);

    SrcAddr srcAddr =  srcAddrMap.get(sAddr);
    if(srcAddr == null) {
      srcAddr = new SrcAddr();
      srcAddrMap.put(sAddr, srcAddr);
    }
    if(isWebSms(srcSme)) {
      srcAddr.isWebSmsMsc = srcMsc;
      dstAddrMap.put(dAddr, srcMsc);
    }else {
      srcAddr.smsxServices.add(serviceId);
    }

    CountKey key = new CountKey(srcSme, srcMsc, serviceId, dstRegion);
    Integer c = count.get(key);
    if(c == null) {
      count.put(key, 1);
    }else {
      count.put(key, c + 1);
    }

  }


  /**
* @author Aleksandr Khalitov
*/
  static class SrcAddr {
  Set<Integer> smsxServices = new HashSet<Integer>(10);
  Boolean isWebSmsMsc;
}

  /**
* @author Aleksandr Khalitov
*/
  private static class CountKey {

  String srcSmeId;
  boolean isMsc;
  int serviceId;
  String region;     //todo optimization?

  CountKey(String srcSmeId, boolean msc, int serviceId, String region) {
    this.srcSmeId = srcSmeId;
    isMsc = msc;
    this.serviceId = serviceId;
    this.region = region;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    CountKey countKey = (CountKey) o;

    if (isMsc != countKey.isMsc) return false;
    if (serviceId != countKey.serviceId) return false;
    if (region != null ? !region.equals(countKey.region) : countKey.region != null) return false;
    if (srcSmeId != null ? !srcSmeId.equals(countKey.srcSmeId) : countKey.srcSmeId != null) return false;

    return true;
  }

  @Override
  public int hashCode() {
    int result = srcSmeId != null ? srcSmeId.hashCode() : 0;
    result = 31 * result + (isMsc ? 1 : 0);
    result = 31 * result + serviceId;
    result = 31 * result + (region != null ? region.hashCode() : 0);
    return result;
  }
}
}
