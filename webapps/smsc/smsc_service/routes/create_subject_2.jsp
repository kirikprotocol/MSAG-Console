<%@ include file="/common/header_begin.jsp"%>
<%
  String name = request.getParameter("name");
  String masks = request.getParameter("masks");
  String sme = request.getParameter("sme");
  Subject s = new Subject(name, masks, smsc.getSmes().get(sme));
  smsc.getSubjects().add(s);
%>
  <script language="JavaScript">
    function updateAndClose()
    {
      if (window.opener != null) {
        var src = window.opener.document.body.all("available_src");
        var dst = window.opener.document.body.all("available_dst");
        var oOptionSrc = window.opener.document.createElement("OPTION");
        oOptionSrc.text="<%=name%>";
        oOptionSrc.value="<%=name%>";
        var oOptionDst = window.opener.document.createElement("OPTION");
        oOptionDst.text="<%=name%>";
        oOptionDst.value="<%=name%>";
  
        src.add(oOptionSrc);
        dst.add(oOptionDst);
        window.close();
      }
    }
  </script>
</head>
<body onload="updateAndClose()">
  Subject created
</body>
</html>