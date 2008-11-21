package mobi.eyeline.smsquiz.quizes;

import ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.Column;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.Row;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.column.TextColumn;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.column.RowControlButtonColumn;
import ru.novosoft.smsc.jsp.util.helper.Validation;

import java.util.*;

import mobi.eyeline.smsquiz.quizes.AnswerCategory;

/**
 * author: alkhal
 * Date: 10.11.2008
 */
public class CategoriesTableHelper extends DynamicTableHelper {

  private Column nameColumn;
  private Column patternColumn;
  private Column answerColumn;

  public CategoriesTableHelper(String name, String uid, int width, Validation validation, boolean allowEditPropsAfterAdd) {
    super(name, uid);
    nameColumn = new TextColumn(this, "smsquiz.label.category.name", uid + "_name", width / 3, validation, allowEditPropsAfterAdd);
    patternColumn = new TextColumn(this, "smsquiz.label.category.pattern", uid + "_pattern", width / 3, validation, allowEditPropsAfterAdd);
    answerColumn = new TextColumn(this, "smsquiz.label.category.answer", uid + "_answer", width / 3, validation, allowEditPropsAfterAdd);
    Column delColumn = new RowControlButtonColumn(this, "", "delColumn");
    addColumn(nameColumn);
    addColumn(patternColumn);
    addColumn(answerColumn);
    addColumn(delColumn);
  }

  protected void fillTable() {
  }

  public List getCategories() {
    final List result = new LinkedList();
    for (Iterator iter = getRows(); iter.hasNext();) {
      Row row = (Row) iter.next();
      String name = (String) (row.getValue(nameColumn));
      String pattern = (String) (row.getValue(patternColumn));
      String answer = (String) (row.getValue(answerColumn));
      result.add(new AnswerCategory(pattern, name, answer));
    }
    return result;
  }

  public void fillCategories(Collection categories) {
    if(categories!=null){
      Iterator iter = categories.iterator();
      while(iter.hasNext()) {
        AnswerCategory cat = (AnswerCategory)iter.next();
        Row row = createNewRow();
        row.addValue(nameColumn,cat.getName());
        row.addValue(patternColumn, cat.getPattern());
        row.addValue(answerColumn, cat.getAnswer());
      }
    }
  }

}
