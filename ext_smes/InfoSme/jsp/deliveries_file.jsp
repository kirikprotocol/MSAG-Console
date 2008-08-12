<%@ page import="ru.novosoft.smsc.infosme.beans.DeliveriesNew"%>
<%@ page import="ru.novosoft.smsc.infosme.beans.deliveries.DeliveriesPageData"%>


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
      <td><input class=check type=checkbox name=splitDeliveriesFile id=splitDeliveriesFile <%=deliveries_bean.isSplitDeliveriesFile() ? "checked" : ""%>></td>
    </tr>
  </table>
</div>

<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel",    "infosme.hint.cancel_task_add", "clickCancel()");
  page_menu_button(session, out, "mbNext",   "infosme.button.next_page", "infosme.hint.next_page", false);
  page_menu_space(out);
  page_menu_end(out);
%>

<script type="text/javascript">
  function checkCreateButton() {
    document.getElementById('mbNext').disabled = (document.getElementById('<%=DeliveriesPageData.ABONENTS_FILE_PARAM%>').value.length <= 0);
    window.setTimeout(checkCreateButton, 5000);
  }
  checkCreateButton();
</script>

