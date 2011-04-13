package ru.sibinco.smsx.stats.backend;

import junit.framework.TestCase;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import ru.sibinco.smsx.stats.backend.SmsxRequest;
import ru.sibinco.smsx.stats.backend.StatisticsException;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import java.util.Date;
import java.util.HashSet;

/**
 * @author Aleksandr Khalitov
 */
public class SmsxRequestTest extends TestCase {

  private static SmsxRequest buildRequest() {
    SmsxRequest request = new SmsxRequest();
    request.setError("sdadsdas");
    request.setFrom(new Date(0));
    request.setTill(new Date(2141241214124l));
    request.setId(12);
    request.setProgress(12123);
    request.setStatus(SmsxRequest.Status.ERROR);
    request.setReportTypesFilter(new HashSet(){{
      add(SmsxRequest.ReportType.SMSX_USERS); add(SmsxRequest.ReportType.WEB_DAILY);
    }});
    request.setServiceIdFilter(new HashSet() {{
      add(new Integer(1));
      add(new Integer(2));
    }});
    return request;
  }

  public void testCopy() {
    SmsxRequest r1 = buildRequest();
    SmsxRequest r2 = r1.copy();
    assertEquals(r1,r2);
  }

  public void testSave() throws ParserConfigurationException, StatisticsException {
    SmsxRequest r1 = buildRequest();
    Document document = DocumentBuilderFactory.newInstance().newDocumentBuilder().newDocument();
    Element el = document.createElement("request");
    r1.save(el, document);
    SmsxRequest r2 = new SmsxRequest();
    r2.load(el);
    assertEquals(r1, r2);
  }

  public void testValidate() throws Exception {
    buildRequest().validate();
  }


}
