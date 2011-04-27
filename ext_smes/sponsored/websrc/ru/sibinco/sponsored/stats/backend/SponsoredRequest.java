package ru.sibinco.sponsored.stats.backend;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import ru.sibinco.sponsored.stats.backend.datasource.ShutdownIndicator;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
public class SponsoredRequest {

  private int id;

  private int progress;

  private String error;

  private Date from;

  private Date till;

  private Status status = Status.IN_PROCESS;

  private float cost;


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

  SponsoredRequest() {
  }

  public float getCost() {
    return cost;
  }

  public void setCost(float cost) {
    this.cost = cost;
  }

  SponsoredRequest copy() {
    SponsoredRequest r = new SponsoredRequest();
    r.id = id;
    r.from = from == null ? null : new Date(from.getTime());
    r.till = till == null ? null : new Date(till.getTime());
    r.status = status;
    r.error = error;
    r.progress = progress;
    r.cost = cost;
    return r;
  }

  void validate() throws StatisticsException {
    if(from == null) {
      throw new StatisticsException("From date is null");
    }
    if(till == null) {
      throw new StatisticsException("Till date is null");
    }
    if(till.before(from)) {
      throw new StatisticsException("Till date is before from date");
    }
  }

  void execute(RequestExecutor executor, ShutdownIndicator shutdownIndicator) throws StatisticsException {
    executor.execute(this, shutdownIndicator);
  }

  void copyFrom(SponsoredRequestPrototype prototype) {
    this.from = prototype.getFrom() == null ? null : new Date(prototype.getFrom().getTime());
    this.till = prototype.getTill() == null ? null : new Date(prototype.getTill().getTime());
    this.cost = prototype.getCost();
  }


  public boolean equals(Object o) {
    if (this == o) return true;
    if (!(o instanceof SponsoredRequest)) return false;

    SponsoredRequest that = (SponsoredRequest) o;

    if (id != that.id) return false;
    if (error != null ? !error.equals(that.error) : that.error != null) return false;
    SimpleDateFormat sdf = new SimpleDateFormat("ddMMyyyy");
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
    if(cost != that.cost)return false;

    return true;
  }

  public String toString() {
    final StringBuffer sb = new StringBuffer();
    sb.append("SponsoredRequest");
    sb.append("{cost=").append(cost);
    sb.append(", ");
    sb.append("id=").append(id);
    sb.append(", progress=").append(progress);
    sb.append(", error='").append(error).append('\'');
    sb.append(", from=").append(from);
    sb.append(", till=").append(till);
    sb.append(", status='").append(status).append('\'');
    sb.append('}');
    return sb.toString();
  }

  void load(Element element) throws StatisticsException {

    final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy");
    {
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
    {
      NodeList nl = element.getElementsByTagName("cost");
      if(nl != null && nl.getLength() > 0) {
        cost = Float.parseFloat(nl.item(0).getFirstChild().getNodeValue());
      }
    }
  }

  void save(Element element, Document document) {
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
    Element el = document.createElement("cost");
    el.appendChild(document.createTextNode(Float.toString(cost)));
    element.appendChild(el);
  }   public int hashCode() {
    int result = id;
    result = 31 * result + (error != null ? error.hashCode() : 0);
    result = 31 * result + (from != null ? from.hashCode() : 0);
    result = 31 * result + (till != null ? till.hashCode() : 0);
    result = 31 * result + (status != null ? status.hashCode() : 0);
    return result;
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