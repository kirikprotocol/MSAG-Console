<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page>
  <jsp:attribute name="title">
    <c:choose>
      <c:when test="${param.add}">Create new provider</c:when>
      <c:otherwise>Edit provider "${param.editId}"</c:otherwise>
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
    <sm-ep:properties title="Provider info">
      <c:if test="${!bean.add}"><sm-ep:txt title="id" name="id" readonly="true"/></c:if>
      <sm-ep:txt title="name" name="name" validation="nonEmpty"/>
    </sm-ep:properties>
  </jsp:body>
</sm:page>