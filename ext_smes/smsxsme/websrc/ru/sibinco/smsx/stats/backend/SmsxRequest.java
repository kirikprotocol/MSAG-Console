package ru.sibinco.smsx.stats.backend;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import ru.sibinco.smsx.stats.backend.datasource.ShutdownIndicator;

import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class SmsxRequest {

  private Set serviceIdFilter;

  private Set reportTypesFilter;  private int id;

  private int progress;

  private String error;

  private Date from;

  private Date till;

  private Status status = Status.IN_PROCESS;

  public Date getFrom() {
    return from;
  }

  void setFrom(Date from) {
    this.from = from;
  }

  public Date getTill() {
    return till;
  }

  void setTill(Date till) {
    this.till = till;
  }

  public Status getStatus() {
    return status;
  }

  public void setStatus(Status status) {
    this.status = status;
  }

  public int getProgress() {
    return progress;
  }

  void setProgress(int progress) {
    this.progress = progress;
  }

  public int getId() {
    return id;
  }

  void setId(int id) {
    this.id = id;
  }

  public String getError() {
    return error;
  }

  void setError(String error) {
    this.error = error;
  }

  SmsxRequest() {
  }

  protected SmsxRequest copy() {
    SmsxRequest r = new SmsxRequest();
    r.id = id;
    r.from = from == null ? null : new Date(from.getTime());
    r.till = till == null ? null : new Date(till.getTime());
    r.status = status;
    r.error = error;
    r.progress = progress;
    if(reportTypesFilter != null) {
      r.reportTypesFilter = new HashSet(reportTypesFilter);
    }
    if(serviceIdFilter != null) {
      r.serviceIdFilter = new HashSet(serviceIdFilter);
    }
    return r;
  }

  void copyFrom(SmsxRequestPrototype prototype) {
    from = prototype.getFrom() == null ? null : new Date(prototype.getFrom().getTime());
    till = prototype.getTill() == null ? null : new Date(prototype.getTill().getTime());
    serviceIdFilter = prototype.getServiceIdFilter() == null ? null : new HashSet(prototype.getServiceIdFilter());
    reportTypesFilter = prototype.getReportTypeFilter() == null ? null : new HashSet(prototype.getReportTypeFilter());
  }

  protected void validate() throws StatisticsException {
    if(from == null) {
      throw new StatisticsException("From date is null");
    }
    if(till == null) {
      throw new StatisticsException("Till date is null");
    }
    if(till.before(from)) {
      throw new StatisticsException("Till date is before from date");
    }
    if(reportTypesFilter != null && reportTypesFilter.isEmpty()) {
      throw new StatisticsException("List of reports is empty");
    }
    if((reportTypesFilter == null || reportTypesFilter.contains(ReportType.SMSX_USERS) || reportTypesFilter.contains(ReportType.TRAFFIC))
        && (serviceIdFilter != null && serviceIdFilter.isEmpty())) {
      throw new StatisticsException("List of services is empty");
    }
  }


  void execute(RequestExecutor executor, ShutdownIndicator shutdownIndicator) throws StatisticsException {
    executor.execute(this, shutdownIndicator);
  }

  protected void save(Element element, Document document) {
    {
      final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

      Element el = document.createElement("id");
      el.appendChild(document.createTextNode(Integer.toString(id)));
//    el.setTextContent(Integer.toString(id));
      element.appendChild(el);

      if(from != null) {
        el = document.createElement("from");
        el.appendChild(document.createTextNode(sdf.format(from)));
//      el.setTextContent(sdf.format(from));
        element.appendChild(el);
      }
      if(till != null) {
        el = document.createElement("till");
        el.appendChild(document.createTextNode(sdf.format(till)));
//      el.setTextContent(sdf.format(till));
        element.appendChild(el);
      }

      el = document.createElement("status");
      el.appendChild(document.createTextNode(status.toString()));
//    el.setTextContent(status.toString());
      element.appendChild(el);

      if(error != null) {
        el = document.createElement("error");
        el.appendChild(document.createTextNode(error));
//      el.setTextContent(error);
        element.appendChild(el);
      }
    }
    if(serviceIdFilter != null) {
      Element el = document.createElement("serviceIds");
      StringBuffer buf = new StringBuffer();
      boolean first = true;
      Iterator i = serviceIdFilter.iterator();
      while(i.hasNext()) {
        if(!first) {
          buf.append(',');
        }
        first = false;
        buf.append(i.next());
      }
      el.appendChild(document.createTextNode(buf.toString()));
      element.appendChild(el);
    }
    if(reportTypesFilter != null) {
      Element el = document.createElement("reportTypes");
      StringBuffer buf = new StringBuffer();
      Iterator i = reportTypesFilter.iterator();
      boolean first = true;
      while(i.hasNext()) {
        if(!first) {
          buf.append(',');
        }
        first = false;
        buf.append(i.next().toString());
      }
      el.appendChild(document.createTextNode(buf.toString()));
      element.appendChild(el);
    }
  }

  protected void load(Element element) throws StatisticsException {
    {


    final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy");

    NodeList nl = element.getChildNodes();
    try{
      for(int i=0; i<nl.getLength(); i++) {
        Node n = nl.item(i);
        if(n.getNodeType() == Node.ELEMENT_NODE) {
          Element e = (Element)n;
          String value = e.getFirstChild().getNodeValue();
          if(e.getTagName().equals("id")) {
            id = Integer.parseInt(value);
          }else if(e.getTagName().equals("from")) {
            from = sdf.parse(value);
          }else if(e.getTagName().equals("till")) {
            till = sdf.parse(value);
          }else if(e.getTagName().equals("error")) {
            error = value;
          }else if(e.getTagName().equals("status")) {
            status = Status.valueOf(value);
          }
        }
      }
    }catch (Exception e) {
      throw new StatisticsException(e);
    }
    }
    NodeList nl = element.getElementsByTagName("serviceIds");
    if(nl != null && nl.getLength() != 0) {
      String[] ss = nl.item(0).getFirstChild().getNodeValue().split(",");
      serviceIdFilter = new HashSet(ss.length);
      for(int i=0; i<ss.length; i++) {
        serviceIdFilter.add(Integer.valueOf(ss[i]));
      }
    }
    nl = element.getElementsByTagName("reportTypes");
    if(nl != null && nl.getLength() != 0) {
      String[] ss = nl.item(0).getFirstChild().getNodeValue().split(",");
      reportTypesFilter = new HashSet(ss.length);
      for(int i=0; i<ss.length;i++) {
        reportTypesFilter.add(ReportType.valueOf(ss[i]));
      }
    }
  }

  public Set getServiceIdFilter() {
    return serviceIdFilter;
  }

  public void setServiceIdFilter(Set serviceIdFilter) {
    this.serviceIdFilter = serviceIdFilter;
  }

  public Set getReportTypesFilter() {
    return reportTypesFilter;
  }

  public void setReportTypesFilter(Set reportTypesFilter) {
    this.reportTypesFilter = reportTypesFilter;
  }

  public String toString() {
    final StringBuffer sb = new StringBuffer();
    sb.append("SmsxRequest");
    sb.append("{serviceId=").append(serviceIdFilter);
    sb.append(", ");
    sb.append("id=").append(id);
    sb.append(", progress=").append(progress);
    sb.append(", error='").append(error).append('\'');
    sb.append(", from=").append(from);
    sb.append(", till=").append(till);
    sb.append(", status='").append(status).append('\'');
    sb.append(", reportTypes=").append(reportTypesFilter == null ? "null" : reportTypesFilter.toString());
    sb.append('}');
    return sb.toString();
  }

  public boolean equals(Object o) {
    if (this == o) return true;
    if (!(o instanceof SmsxRequest)) return false;

    SmsxRequest that = (SmsxRequest) o;

    SimpleDateFormat sdf = new SimpleDateFormat("ddMMyyyy");

    if (reportTypesFilter != null ? !reportTypesFilter.equals(that.reportTypesFilter) : that.reportTypesFilter != null) return false;
    if (serviceIdFilter != null ? !serviceIdFilter.equals(that.serviceIdFilter) : that.serviceIdFilter != null) return false;
    if (id != that.id) return false;
    if (error != null ? !error.equals(that.error) : that.error != null) return false;
    if(till != null) {
      if(that.till == null || !sdf.format(till).equals(sdf.format(that.till))) {
        return false;
      }
    }else {
      if(that.till != null) {
        return false;
      }
    }
    if(from != null) {
      if(that.from == null || !sdf.format(from).equals(sdf.format(that.from))) {
        return false;
      }
    }else {
      if(that.from != null) {
        return false;
      }
    }
    if (status != null ? !status.equals(that.status) : that.status != null) return false;

    return true;
  }


  public static class ReportType {

    public static final ReportType SMSX_USERS = new ReportType("SMSX_USERS");
    public static final ReportType WEB_DAILY = new ReportType("WEB_DAILY");
    public static final ReportType TRAFFIC = new ReportType("TRAFFIC");
    public static final ReportType WEB_REGIONS = new ReportType("WEB_REGIONS");

    private static final Map map = new HashMap(4){{
      put(SMSX_USERS.s, SMSX_USERS);
      put(WEB_DAILY.s, WEB_DAILY);
      put(WEB_REGIONS.s, WEB_REGIONS);
      put(TRAFFIC.s, TRAFFIC);
    }};

    private final String s;

    private ReportType(String s) {
      this.s = s;
    }

    public String toString() {
      return s;
    }

    public static ReportType valueOf(String str) {
      return (ReportType) map.get(str);
    }

    public static ReportType[] values() {
      ReportType[] types = new ReportType[map.size()];
      Iterator i = map.values().iterator();
      int j = 0;
      while(i.hasNext()) {
        types[j] = (ReportType)i.next();
        j++;
      }
      return types;
    }
  }


  public static class Status {

    public static final Status IN_PROCESS = new Status("IN_PROCESS");
    public static final Status CANCELED = new Status("CANCELED");
    public static final Status READY = new Status("READY");
    public static final Status ERROR = new Status("ERROR");

    private static final Map map = new HashMap(4){{
      put(IN_PROCESS.s, IN_PROCESS);
      put(CANCELED.s, CANCELED);
      put(READY.s, READY);
      put(ERROR.s, ERROR);
    }};

    private final String s;

    private Status(String s) {
      this.s = s;
    }

    public String toString() {
      return s;
    }

    public static Status valueOf(String str) {
      return (Status) map.get(str);
    }

    public static Status[] values() {
      Status[] r = new Status[map.size()];
      Iterator i = map.values().iterator();
      int j=0;
      while(i.hasNext()) {
        r[j] = (Status)i.next();
        j++;
      }
      return r;
    }
  }


}
