package ru.novosoft.smsc.admin.console.commands.misc;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.category.Category;

import java.util.Iterator;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 12.04.2005
 * Time: 14:47:27
 * To change this template use File | Settings | File Templates.
 */
public class CategoryListCommand  implements Command
{
    public void process(CommandContext ctx)
    {
        try {
            CategoryManager manager = ctx.getCategoryManager();
            Iterator i = manager.getCategories().keySet().iterator();
            if (!i.hasNext()) {
                ctx.setMessage("No categories defined");
                ctx.setStatus(CommandContext.CMD_OK);
            } else {
                while (i.hasNext()) {
                    Long categoryId = (Long)i.next();
                    Category category = manager.getCategory(categoryId);
                    if (category != null) {
                        String categoryName = (category.getName() != null) ? category.getName():"";
                        ctx.addResult(""+categoryId.longValue()+" '"+categoryName+"'");
                    }
                }
                ctx.setMessage("Categories list");
                ctx.setStatus(CommandContext.CMD_LIST);
            }
        } catch (Exception e) {
            ctx.setMessage("Couldn't list categories. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "CATEGORY_LIST";
    }
}
