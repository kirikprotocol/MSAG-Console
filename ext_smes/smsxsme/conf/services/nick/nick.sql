nick.update = UPDATE nicks SET nick=? WHERE address=?
nick.insert = INSERT INTO nicks (nick, address) VALUES (?,?)
nick.delete = DELETE FROM nicks WHERE address=?
nick.load.by.address = SELECT address, nick FROM nicks WHERE address=?