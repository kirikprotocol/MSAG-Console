<%@ page import="ru.novosoft.smsc.infosme.beans.Deliveries"%><jsp:useBean id="deliveries_bean" class="ru.novosoft.smsc.infosme.beans.Deliveries" scope="session"/><jsp:setProperty name="deliveries_bean" property="*"/><%
    ru.novosoft.smsc.infosme.beans.Deliveries bean = deliveries_bean;
    
    bean.process(request);
    out.println("status, messages, progress");
    String color = (bean.getStatus() == Deliveries.STATUS_OK) ? "green":
                   ((bean.getStatus() == Deliveries.STATUS_ERR) ? "red":"blue");
    out.print("\"<span style='color:"+color+";'>"+bean.getStatusStr()+"</span>\"");
    out.print(", ");
    out.print("\"<span>"+bean.getMessages()+"</span>\"");
    out.print(", ");
    out.print("\"<span>"+bean.getProgress()+"</span>\"");
%>