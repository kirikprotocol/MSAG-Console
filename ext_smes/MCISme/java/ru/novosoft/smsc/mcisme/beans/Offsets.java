package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 20.07.2005
 * Time: 17:14:33
 * To change this template use File | Settings | File Templates.
 */
public class Offsets extends MCISmeBean
{
    private String editOffset = null;

    private String[]   checked    = new String[0];
    private Collection checkedSet = null;

    private String mbAdd = null;
    private String mbEdit = null;
    private String mbDelete = null;

    public class Identity {
      public int    offset = 0;
      public String name = "";

      public Identity() {}
      public Identity(int offset, String name) {
        this.offset = offset; this.name = name;
      }
    }
    private class IdentityComparator implements Comparator
    {
      public int compare(Object o1, Object o2) {
        if (o1 != null && o1 instanceof Offsets.Identity && o2 != null && o2 instanceof Offsets.Identity) {
          Offsets.Identity i1 = (Offsets.Identity)o1; Offsets.Identity i2 = (Offsets.Identity)o2;
          return (i2.offset-i1.offset);
        }
        return 0;
      }
    }

    private SortedList offsets = new SortedList(new IdentityComparator());

    protected int init(List errors)
    {
      int result = super.init(errors);
      if (result != RESULT_OK)  return result;

      checkedSet = new HashSet(Arrays.asList(checked));
      return result;
    }

    public int process(HttpServletRequest request)
    {
      int result = super.process(request);
      if (result != RESULT_OK) return result;

      if      (mbAdd != null)    return RESULT_OFFSET_ADD;
      else if (mbEdit != null)   return RESULT_OFFSET_EDIT;
      else if (mbDelete != null) result = deleteOffsets();

      int loadResult = loadupFromConfig(getConfig());
      return (result != RESULT_OK) ? result:loadResult;
    }

    private int loadupFromConfig(Config config)
    {
        try {
          Set set = config.getSectionChildShortSectionNames(OFFSETS_SECTION_NAME);
          for (Iterator i = set.iterator(); i.hasNext();) {
            String section = (String)i.next();
            int offset = config.getInt(OFFSETS_SECTION_NAME + '.' + section + ".offset");
            offsets.add(new Offsets.Identity(offset, section));
          }
        } catch (Exception e) {
          return error("Failed to load TZ offsets", e);
        }
        return RESULT_OK;
    }
    private int deleteOffsets()
    {
        try
        {
          final String offsetPrefix = OFFSETS_SECTION_NAME + '.';
          for (int i=0; i<checked.length; i++)
          {
            String section = offsetPrefix + StringEncoderDecoder.encodeDot(checked[i]);
            getConfig().removeSection(section);
            getMCISmeContext().setChangedOffsets(true);
            checkedSet.remove(checked[i]);
          }
        } catch (Exception e) {
          return error("Failed to delete offset(s)", e);
        }
        return RESULT_OK;
    }

    public boolean isChecked(String offsetName) {
      return checkedSet.contains(offsetName);
    }
    public String[] getChecked() {
      return checked;
    }
    public void setChecked(String[] checked) {
      this.checked = checked;
    }
    public Collection getCheckedSet() {
      return checkedSet;
    }
    public List getOffsets() {
      return offsets;
    }

    public String getEditOffset() {
      return editOffset;
    }
    public void setEditOffset(String editOffset) {
      this.editOffset = editOffset;
    }

    public String getMbAdd() {
        return mbAdd;
    }
    public void setMbAdd(String mbAdd) {
        this.mbAdd = mbAdd;
    }

    public String getMbEdit() {
        return mbEdit;
    }
    public void setMbEdit(String mbEdit) {
        this.mbEdit = mbEdit;
    }

    public String getMbDelete() {
        return mbDelete;
    }
    public void setMbDelete(String mbDelete) {
        this.mbDelete = mbDelete;
    }
}
