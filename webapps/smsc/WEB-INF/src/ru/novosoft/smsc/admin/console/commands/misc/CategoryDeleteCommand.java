package ru.novosoft.smsc.admin.console.commands.misc;

import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 12.04.2005
 * Time: 14:46:34
 * To change this template use File | Settings | File Templates.
 */
public class CategoryDeleteCommand extends CommandClass
{
    protected String categoryId = null;

    public void process(CommandContext ctx)
    {
        String out = "Category '"+categoryId+"'";
        try {
            CategoryManager manager = ctx.getCategoryManager();
            Category category = manager.getCategoryByName(categoryId);
            if (category == null) throw new Exception("Category not exists");
            manager.removeCategory(categoryId);
            ctx.setMessage(out+" deleted");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Couldn't delete "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public void setCategoryId(String categoryId) {
        this.categoryId = categoryId;
    }

    public String getId() {
        return "CATEGORY_DELETE";
    }

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		journalAppend(ctx, userName, SubjectTypes.TYPE_category, categoryId, Actions.ACTION_DEL);
		ctx.getStatuses().setCategoriesChanged(true);
	}
}

