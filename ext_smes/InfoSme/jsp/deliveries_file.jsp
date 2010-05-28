<%@ page import="ru.novosoft.smsc.infosme.beans.deliveries.Deliveries"%>
<%@ page import="ru.novosoft.smsc.infosme.beans.deliveries.DeliveriesPageData"%>

<input type="hidden" name="jsp" value="<%=CPATH%>/smsc/esme_InfoSme/deliveries_new.jsp?mbNext='1'">

<div class=content>
  <div class=page_subtitle><%= getLocString("infosme.subtitle.stage1")%></div><br/>
  <table class=properties_list cellspacing=0 cellpadding=0>
    <col width="15%" align=right>
    <col width="85%">
    <tr class=row0>
      <th><%= getLocString("infosme.label.file_location")%></th>
      <td><input class=txt type=file name="<%=DeliveriesPageData.ABONENTS_FILE_PARAM%>" id="<%=DeliveriesPageData.ABONENTS_FILE_PARAM%>" onChange="checkCreateButton();"></td>
    </tr>
    <tr class=row1>
      <th><%= getLocString("infosme.label.split_file")%></th>
      <td><input class=check type=checkbox name="splitDeliveriesFile" id="splitDeliveriesFile" <%=deliveries_bean.isSplitDeliveriesFile() ? "checked" : ""%> onchange="checkSplitFile()"></td>
    </tr>
    <tr class=row0>
      <th><%= getLocString("infosme.label.use_same_settings_for_all_regions")%></th>
      <td><input class=check type=checkbox name="useSameSettingsForAllRegions" id="useSameSettingsForAllRegions" <%=deliveries_bean.isUseSameSettingsForAllRegions() ? "checked" : ""%>></td>
    </tr>
  </table>
</div>

<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel",    "infosme.hint.cancel_task_add", "clickCancel()");
  page_menu_button(session, out, "mbNext",   "infosme.button.next_page", "infosme.hint.next_page", "return clickNext();", false);
  page_menu_space(out);
  page_menu_end(out);
%>

<script type="text/javascript">
  function checkCreateButton() {
    document.getElementById('mbNext').disabled = (document.getElementById('<%=DeliveriesPageData.ABONENTS_FILE_PARAM%>').value.length <= 0);
  }

  function checkSplitFile() {
    document.getElementById('useSameSettingsForAllRegions').disabled = !document.getElementById('splitDeliveriesFile').checked
  }

  function clickNext() {
    opForm.action="<%=CPATH+"/upload"%>";
    opForm.method="POST";
    opForm.enctype="multipart/form-data";
    opForm.setAttribute("encoding", "multipart/form-data");
    opForm.submit();
    return true;
  }

  checkSplitFile()
</script>

