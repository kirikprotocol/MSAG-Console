<%@include file="/WEB-INF/inc/header.jspf"%>
<script src="tree.js"></script>
<sm:page title="logging.title">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbSave" value="logging.item.mbsave.value" title="logging.item.mbsave.title"/>
      <sm-pm:item name="mbCancel" value="logging.item.mbcancel.value" title="logging.item.mbcancel.title" onclick="clickCancel()"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>
  <jsp:body>
    Place your content here
 </jsp:body>
</sm:page>