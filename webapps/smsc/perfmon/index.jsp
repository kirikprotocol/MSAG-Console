<%@include file="../WEB-INF/inc/code_header.jsp"  %>
<%@include file="../WEB-INF/inc/html_3_header.jsp"  %>
<%=request.getServerName()%>


      <applet code="ru.novosoft.smsc.perfmon.applet.PerfMon.class" width=600 height=400 archive="perfmon.jar">
        <param name="locale.country" value="ru">
        <param name="locale.language" value="ru">
        <param name="pixPerSecond" value="4">
        <param name="scale" value="160">
        <param name="block" value="8">
        <param name="vLightGrid" value="4">
        <param name="vMinuteGrid" value="6">
        <param name="port" value="9604">
        <param name="host" value="<%=request.getServerName()%>">
      </applet>
<%@include file="../WEB-INF/inc/html_3_footer.jsp"  %>
<%@include file="../WEB-INF/inc/code_footer.jsp"  %>

