<%@ page import="java.util.Iterator"%><%@ page import="ru.novosoft.smsc.infosme.beans.DeliveriesNew"%><jsp:useBean id="deliveries_bean" scope="session" class="ru.novosoft.smsc.infosme.beans.DeliveriesNew" /><%
  deliveries_bean.process(request);
  out.print("status, recordsProcessed, subjectsFound, unrecognized, inBlackList");
  out.print(" \r\n");
  out.print(deliveries_bean.getProcessFileStatusStr());
  out.print(", " + deliveries_bean.getRecordsProcessed());
  out.print(", " + deliveries_bean.getSubjectsFound());
  out.print(", " + deliveries_bean.getUnrecognized());
  out.print(", " + deliveries_bean.getInBlackList());
%>