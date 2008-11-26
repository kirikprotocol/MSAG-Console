package mobi.eyeline.smsquiz.quizes.view;

import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.novosoft.smsc.jsp.util.helper.statictable.Row;
import ru.novosoft.smsc.jsp.util.helper.statictable.OrderType;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.CheckBoxCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.cell.StringCell;
import ru.novosoft.smsc.jsp.util.helper.statictable.column.TextColumn;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.DataSource;

import javax.servlet.http.HttpServletRequest;
import java.util.Date;
import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;
import java.text.SimpleDateFormat;

import mobi.eyeline.smsquiz.quizes.view.QuizesDataSource;
import mobi.eyeline.smsquiz.quizes.view.QuizQuery;

/**
 * author: alkhal
 * Date: 07.11.2008
 */
public class QuizesStaticTableHelper extends PagedStaticTableHelper {

  private final TextColumn checkColumn = new TextColumn("checkColumn", "", false);
  private final TextColumn quizidColumn = new TextColumn(QuizesDataSource.QUIZ_ID, "smsquiz.quiz.quizId", true, 25);
  private final TextColumn dateBeginColumn = new TextColumn(QuizesDataSource.DATE_BEGIN, "smsquiz.quiz.date.begin", true, 25);
  private final TextColumn dateEndColumn = new TextColumn(QuizesDataSource.DATE_END, "smsquiz.quiz.date.end", true, 25);
  private final TextColumn stateColumn = new TextColumn(QuizesDataSource.STATE, "smsquiz.quiz.state", true, 20);

  private static final SimpleDateFormat DATE_FORMAT = new SimpleDateFormat("dd.MM.yy HH:mm");

  private DataSource ds;

  private int totalSize = 0;

  private String sortOrder = "";


  public QuizesStaticTableHelper(String uid) {
    super(uid, false);

    addColumn(checkColumn);
    addColumn(quizidColumn);
    addColumn(dateBeginColumn);
    addColumn(dateEndColumn);
    addColumn(stateColumn);
  }

  private void buildSortOrder() {
    SortOrderElement[] sortOrderElements = getSortOrder();
    if ((sortOrderElements != null) && (sortOrderElements.length > 0)) {
      SortOrderElement element = sortOrderElements[0];
      if (element != null) {
        sortOrder = element.getColumnId();
        if (sortOrder != null) {
          if (element.getOrderType() != OrderType.ASC) {
            sortOrder = "-" + sortOrder;
          }
          return;
        }
      }
    }
    sortOrder = QuizesDataSource.QUIZ_NAME;
  }

  protected void fillTable(int start, int size) throws TableHelperException {
    try {
      buildSortOrder();
      final QueryResultSet quizesList = ds.query(new QuizQuery(size, sortOrder, start));

      for (int i = 0; i < quizesList.size(); i++) {
        final DataItem item = quizesList.get(i);

        final Row row = createNewRow();

        final String quizId = (String) item.getValue(QuizesDataSource.QUIZ_ID);
        final String quizName = (String) item.getValue(QuizesDataSource.QUIZ_NAME);
        row.addCell(checkColumn, new CheckBoxCell("chb" + quizId, false));
        row.addCell(quizidColumn, new StringCell(quizId, quizName, true));
        row.addCell(dateBeginColumn, new StringCell(quizId,
            convertDateToString((Date) item.getValue(QuizesDataSource.DATE_BEGIN)), false));
        row.addCell(dateEndColumn, new StringCell(quizId,
            convertDateToString((Date) item.getValue(QuizesDataSource.DATE_END)), false));
        row.addCell(stateColumn, new StringCell(quizId,
            (String) item.getValue(QuizesDataSource.STATE), false));
      }
      totalSize = quizesList.getTotalSize();

    } catch (Exception e) {
      throw new TableHelperException(e);
    }
  }

  public List getSelectedQuizesList(HttpServletRequest request) {
    final ArrayList result = new ArrayList();
    for (Iterator iter = request.getParameterMap().keySet().iterator(); iter.hasNext();) {
      final String paramName = (String) iter.next();
      if (paramName.startsWith("chb"))
        result.add(paramName.substring(3));
    }
    return result;
  }

  protected int calculateTotalSize() throws TableHelperException {
    return totalSize;
  }

  public int getTotalSize() {
    return totalSize;
  }

  public void setTotalSize(int totalSize) {
    this.totalSize = totalSize;
  }

  private String convertDateToString(Date date) {
    return DATE_FORMAT.format(date);
  }


  public void setDataSource(DataSource ds) {
    this.ds = ds;
  }

}
