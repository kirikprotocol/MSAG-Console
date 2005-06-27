package ru.novosoft.smsc.admin.console.commands.misc;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 12.04.2005
 * Time: 14:45:28
 * To change this template use File | Settings | File Templates.
 */

public class CategoryAddCommand implements Command
{
    protected String categoryId = null;

    public void process(CommandContext ctx)
    {
        String out = "Category '"+categoryId+"'";
        try {
            CategoryManager manager = ctx.getCategoryManager();
            Category category = manager.getCategoryByName(categoryId);
            if (category != null) throw new Exception("Category already exists");
            manager.createCategory(categoryId);
            ctx.setMessage(out+" added");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Couldn't add "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public void setCategoryId(String categoryId) {
        this.categoryId = categoryId;
    }

    public String getId() {
        return "CATEGORY_ADD";
    }
}

