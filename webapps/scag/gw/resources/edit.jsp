<%@include file="/WEB-INF/inc/header.jspf"%><%@ taglib prefix="sm-eet" tagdir="/WEB-INF/tags/edit/edit_tree"%>
<sm:page>
  <jsp:attribute name="title">
    <c:choose>
      <c:when test="${param.add}">Create new locale resource"</c:when>
      <c:otherwise>Edit locale resource "${param.editId}"</c:otherwise>
    </c:choose>
  </jsp:attribute>

  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbSave" value="Save" title="Save resource info"/>
      <sm-pm:item name="mbCancel" value="Cancel" title="Cancel resource editing" onclick="clickCancel()"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
  qweqwe
    <sm-eet:section title="Section 1" name="sec1">
      asdasd
    <sm-eet:section title="Section 2" name="sec2">
      dddddddd
    </sm-eet:section>
    </sm-eet:section>
  </jsp:body>
</sm:page>