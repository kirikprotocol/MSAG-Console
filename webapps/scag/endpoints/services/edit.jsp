<%@include file="/WEB-INF/inc/header.jspf"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<sm:page>
  <jsp:attribute name="title">
    <c:choose>
      <c:when test="${param.add}">sp.add.title</c:when>
      <c:otherwise>sp.edit.title</c:otherwise>
    </c:choose>
  </jsp:attribute>

  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbSave" value="sp.edit.item.mbsave.value" title="sp.edit.item.mbsave.title"/>
      <sm-pm:item name="mbCancel" value="sp.edit.item.mbcancel.value" title="sp.edit.item.mbcancel.title" onclick="clickCancel()"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
  <script>
   function changeTransportId(){
      var opForm = getElementByIdUni("opForm");
//      var transport = opForm.all.transportId.options[opForm.all.transportId.selectedIndex].value;
      var selectTransportId = getElementByIdUni("transportId");
      var transport = selectTransportId.options[selectTransportId.selectedIndex].value;
      opForm.submit();
      return true;
    }
    </script>

    <%--<sm-ep:properties title="">
        <c:set var="transportIds" value=""/>
        <c:forEach items="${bean.transportIds}" var="transportId">
            <c:if test="${transportId != '2'}">
                <c:set var="transportIds" value="${transportIds}${transportId},"/>
            </c:if>
        </c:forEach>
        <c:if test="${fn:length(transportIds)>0}">
            <c:set var="transportIds" value="${fn:substring(transportIds, 0, fn:length(transportIds)-1)}"/>
        </c:if>

        <c:set var="transportTitles" value=""/>
        <c:forEach items="${bean.transportTitles}" var="transportTitle">
            <c:if test="${transportTitle != 'HTTP'}">
                <c:set var="transportTitles" value="${transportTitles}${transportTitle},"/>
            </c:if>
        </c:forEach>
        <c:if test="${fn:length(transportTitles)>0}">
            <c:set var="transportTitles" value="${fn:substring(transportTitles, 0, fn:length(transportTitles)-1)}"/>
        </c:if>

        <sm-ep:list title="sp.edit.list.transportid" onChange="changeTransportId();" name="transportId" values="${transportIds}" valueTitles="${transportTitles}"/>
    </sm-ep:properties>
    <br>
    <br>
    <br> --%>
    <input type="hidden" id="transportId" name="transportId" value="1"/>
    <c:choose>
        <c:when test="${bean.transportId == 1}">
            <sm-ep:properties title="sp.edit.properties.smpp_info">
            <br>
            <sm-ep:txt title="sp.edit.txt.id" name="id" maxlength="15" validation="id"/>
            <sm-ep:txt title="sp.edit.txt.timeout" name="timeout" maxlength="6" validation="unsigned"/>
            <sm-ep:txt title="sp.edit.txt.password" name="password"/>
            <sm-ep:check title="sp.edit.check.enabled.title" head="sp.edit.check.enabled.head" name="enabled"/>
            <sm-ep:list title="sp.edit.list.mode" name="mode" values="1,2,3" valueTitles="TX,RX,TRX"/>
            <sm-ep:txtEmpty title="sp.edit.txt.inQueueLimit" name="inQueueLimit" maxlength="6" validation="unsignedOrEmpty"/>
            <sm-ep:txtEmpty title="sp.edit.txt.outQueueLimit" name="outQueueLimit" maxlength="6" validation="unsignedOrEmpty"/>
            <sm-ep:txtEmpty title="sp.edit.txt.maxSmsPerSec" name="maxSmsPerSec" maxlength="6" validation="unsignedOrEmpty"/>
            <sm-ep:check title="sp.edit.check.snmptracking.title" head="sp.edit.check.snmptracking.head" name="snmpTracking"/>
            </sm-ep:properties>
        </c:when>
        <c:when test="${bean.transportId == 2}">
            <sm-ep:properties title="sp.edit.properties.http_info">
            Place your content here
            </sm-ep:properties>
        </c:when>
        <c:when test="${bean.transportId == 3}">
            <sm-ep:properties title="sp.edit.properties.mms_info">
            Place your content here
            </sm-ep:properties>
        </c:when>
    </c:choose>
  </jsp:body>
</sm:page>