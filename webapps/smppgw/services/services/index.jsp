<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Services">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="Add" title="Register new SME"/>
      <sm-pm:item name="mbDelete" value="Delete" title="Unregister checked SME(s)"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm:table columns="checkbox,id,providerName,smsc,wantAlias" names="c,id,provider,is SMSC,wantAlias" widths="1,49,48,1,1" edit="id"/>

    <table class=list cellspacing=1 width="100%">
   <%-- <col width="1%" align=center> --%>
    <%--if (request.isUserInRole("services")) {%><col width="1%"><%}--%>
    <col align=left>
    <%--if (request.isUserInRole("services")) {%><col width="1%" align=left><%}--%>
    <col width="1%" align=center>
   <%-- <col width="1%" align=center>
    <col width="1%" align=center>  --%>
    <thead>
   <tr>
    <%--	<th class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
    	<%--if (request.isUserInRole("services")) {%><th>&nbsp;</th><%}--%>
    	<th>service</th>
    	<%--if (request.isUserInRole("services")) {%><th>host</th><%}--%>
    	<th colspan="1">status</th>
    </tr>
    </thead>
    <tbody>
    <c:set var="rowN" value="0"/>

 <c:forEach items="${bean.smeIds}" var="result" >
 <c:choose>
 <c:when test="${rowN == 0}" ><c:set var="rowN" value="1"/></c:when>
 <c:otherwise><c:set var="rowN" value="0"/></c:otherwise>
 </c:choose>
<c:set var="serviceId" value="${result}"/>
<c:set var="context" value="${bean.appContext}"/>
    <tr class=row${rowN}>
     <td class=name>${serviceId}</td>
     <td class=name>${smf:smeStatus(context,serviceId)}</td>
    </tr>
</c:forEach>
    </tbody>
    </table>
  </jsp:body>
</sm:page>