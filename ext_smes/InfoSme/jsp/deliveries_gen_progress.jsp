<%@ page import="java.util.Iterator"%><%@ page import="ru.novosoft.smsc.infosme.beans.deliveries.Deliveries"%><jsp:useBean id="deliveries_bean" scope="session" class="ru.novosoft.smsc.infosme.beans.deliveries.Deliveries" /><%
  deliveries_bean.process(request);
  out.print("status");
  for (Iterator iter = deliveries_bean.getRegionIds().iterator(); iter.hasNext();) {
    int regionId = ((Integer)iter.next()).intValue();
    out.print(", messages" + regionId + ", progress" + regionId);
  }
  out.print(" \r\n");

  out.print(deliveries_bean.getStatusStr());

  for (Iterator iter = deliveries_bean.getRegionIds().iterator(); iter.hasNext();) {
    int regionId = ((Integer)iter.next()).intValue();
    out.print(", " + deliveries_bean.getMessages(regionId) + ", " + deliveries_bean.getProgress(regionId));
  }
%>