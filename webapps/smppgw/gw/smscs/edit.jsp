<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page>
  <jsp:attribute name="title">
    <c:choose>
      <c:when test="${param.add}">Create new service center"</c:when>
      <c:otherwise>Edit service center "${param.editId}"</c:otherwise>
    </c:choose>
  </jsp:attribute>

  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbSave" value="Save" title="Save provider info"/>
      <sm-pm:item name="mbCancel" value="Cancel" title="Cancel provider editing" onclick="clickCancel()"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <c:set var="smes" value="${fn:join(bean.smes, ',')}"/>
    <sm-ep:properties title="Provider info">
      <sm-ep:list title="name" name="id" values="${smes}" valueTitles="${smes}" readonly="${!bean.add}"/>
      <sm-ep:txt title="host" name="host" validation="nonEmpty"/>
      <sm-ep:txt title="port" name="port" validation="port"/>
      <sm-ep:txt title="systemId" name="systemId" validation="nonEmpty"/>
      <sm-ep:txt title="password" name="password"/><!--type="password"-->
      <sm-ep:txt title="responseTimeout" name="responseTimeout" validation="positive"/>
      <sm-ep:txt title="uniqueMsgIdPrefix" name="uniqueMsgIdPrefix" validation="nonEmpty"/>
    </sm-ep:properties>
  </jsp:body>
</sm:page>