<%@ page import="java.util.Iterator"%><%@ page import="ru.novosoft.smsc.infosme.beans.DeliveriesNew"%><jsp:useBean id="deliveries_bean" scope="session" class="ru.novosoft.smsc.infosme.beans.DeliveriesNew" /><%  
  deliveries_bean.process(request);
  out.print("status");
  for (Iterator iter = deliveries_bean.getSubjects().iterator(); iter.hasNext();) {
    String subject = (String)iter.next();
    out.print(", messages" + subject + ", progress" + subject);
  }
  out.print(" \r\n");

  out.print(deliveries_bean.getStatusStr());

  for (Iterator iter = deliveries_bean.getSubjects().iterator(); iter.hasNext();) {
    String subject = (String)iter.next();
    out.print(", " + deliveries_bean.getMessages(subject) + ", " + deliveries_bean.getProgress(subject));
  }
%>