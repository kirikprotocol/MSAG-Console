<%@ page import="java.util.*, ru.novosoft.smsc.util.StringEncoderDecoder"%><jsp:useBean id="deliveries_bean" class="ru.novosoft.smsc.infosme.beans.Deliveries" scope="session"/><jsp:setProperty name="deliveries_bean" property="*"/><%
    ru.novosoft.smsc.infosme.beans.Deliveries bean = deliveries_bean;

    bean.process(request);
    out.println("status, messages, progress");
    String color = (bean.getStatus() == bean.STATUS_OK) ? "green":
                   ((bean.getStatus() == bean.STATUS_ERR) ? "red":"blue");
    out.print("\"<span style='color:"+color+";'>"+bean.getStatusStr()+"</span>\"");
    out.print(", ");
    out.print("\"<span>"+bean.getMessages()+"</span>\"");
    out.print(", ");
    out.print("\"<span>"+bean.getProgress()+"</span>\"");
%>