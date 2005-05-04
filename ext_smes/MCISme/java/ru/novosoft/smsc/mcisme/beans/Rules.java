package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 04.05.2005
 * Time: 16:01:42
 * To change this template use File | Settings | File Templates.
 */
public class Rules extends MCISmeBean
{
    private String editRule = null;

    private String[]   checked    = new String[0];
    private Collection checkedSet = null;

    private String mbAdd = null;
    private String mbEdit = null;
    private String mbDelete = null;

    public class Identity {
      public int    priority = 0;
      public String name = "";

      public Identity() {}
      public Identity(int priority, String name) {
        this.priority = priority; this.name = name;
      }
    }
    private class IdentityComparator implements Comparator
    {
      public int compare(Object o1, Object o2) {
        if (o1 != null && o1 instanceof Rules.Identity && o2 != null && o2 instanceof Rules.Identity) {
          Rules.Identity i1 = (Rules.Identity)o1; Rules.Identity i2 = (Rules.Identity)o2;
          return (i2.priority-i1.priority);
        }
        return 0;
      }
    }

    private SortedList rules = new SortedList(new IdentityComparator());

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

      if      (mbAdd != null)    return RESULT_RULE_ADD;
      else if (mbEdit != null)   return RESULT_RULE_EDIT;
      else if (mbDelete != null) result = deleteRules();

      int loadResult = loadupFromConfig(getConfig());
      return (result != RESULT_OK) ? result:loadResult;
    }

    private int loadupFromConfig(Config config)
    {
        try {
          Set set = config.getSectionChildShortSectionNames(RULES_SECTION_NAME);
          for (Iterator i = set.iterator(); i.hasNext();) {
            String section = (String)i.next();
            int priority = config.getInt(RULES_SECTION_NAME + '.' + section + ".priority");
            rules.add(new Rules.Identity(priority, section));
          }
        } catch (Exception e) {
          return error("Failed to load rules", e);
        }
        return RESULT_OK;
    }
    private int deleteRules()
    {
        try
        {
          final String rulesPrefix = RULES_SECTION_NAME + '.';
          for (int i=0; i<checked.length; i++)
          {
            String section = rulesPrefix + StringEncoderDecoder.encodeDot(checked[i]);
            getConfig().removeSection(section);
            getMCISmeContext().setChangedRules(true);
            checkedSet.remove(checked[i]);
          }
        } catch (Exception e) {
          return error("Failed to delete rules(s)", e);
        }
        return RESULT_OK;
    }

    public boolean isChecked(String ruleName) {
      return checkedSet.contains(ruleName);
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
    public List getRules() {
      return rules;
    }

    public String getEditRule() {
      return editRule;
    }
    public void setEditRule(String editRule) {
      this.editRule = editRule;
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
