package ru.novosoft.smsc.jsp.smsc.categories;

/**
 * Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.jsp.SMSCErrors;

import java.util.List;

public class CategoriesAdd extends CategoriesEditBean
{
  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (name == null) {
      name = "";
      setRoles(new String[0]);

    }
    return RESULT_OK;
  }

  protected int save()
  {
    if (name == null || name.trim().length() == 0)
      return error(SMSCErrors.error.categories.nameNotDefined);

    Category category = null;
    try {
      category = categoryManager.getCategoryByName(name);
    } catch (Exception e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    }
    if (category == null) {
      category = categoryManager.createCategory(name);
      if (category != null) {
        journalAppend(SubjectTypes.TYPE_category, name, Actions.ACTION_ADD);
        appContext.getStatuses().setCategoriesChanged(true);
        return RESULT_DONE;
      }
      else
        return error(SMSCErrors.error.unknown);
    }
    else {
      return error(SMSCErrors.error.categories.categoryAlreadyExists, name);
    }
  }

  public boolean isNew()
  {
    return true;
  }
}
