<jsp:useBean id="bean" scope="session" class="ru.novosoft.smsc.infosme.beans.Tasks"/><%
  bean.process(request);
  out.print("progress");
  out.print(" \r\n");
  out.print(bean.getProgress()+"%");
%>