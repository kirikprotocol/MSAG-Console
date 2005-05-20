package ru.novosoft.smsc.jsp.smsc.categories;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.jsp.SMSCErrors;

import java.util.List;

public class CategoriesEdit extends CategoriesEditBean
{
  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (name == null || name.trim().length() == 0) {
      //  setRoles(new String[0]);
      name = "";
      return error(SMSCErrors.error.categories.nameNotDefined);
    }
    else {
/*        Provider provider = providerManager.getProviderByName(name);
        id=String.valueOf(provider.getId());
        if ((provider == null) && (mbSave == null)) {
          return error(SMSCErrors.error.providers.providerNotFound, name);
        }
*/
    }

    return RESULT_OK;
  }

  protected int save()
  {
    if (name == null || name.trim().length() == 0)
      return error(SMSCErrors.error.categories.nameNotDefined);
    //Long idl = Long.getLong(id);
	Long idl = Long.decode(id);  
    Category category = categoryManager.getCategory(idl);
    if (category == null) { // add new category
      return error(SMSCErrors.error.categories.categoryNotFound, name);
    }
    else {

      category.setName(name);
      journalAppend(SubjectTypes.TYPE_category, name, Actions.ACTION_MODIFY);
      appContext.getStatuses().setCategoriesChanged(true);
      return RESULT_DONE;
    }
  }

  public boolean isNew()
  {
    return false;
  }

}
