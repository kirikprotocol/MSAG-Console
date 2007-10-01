sender.message.load.max.id = SELECT max(id) FROM smpp_messages

sender.message.load.by.id = SELECT id, smpp_status, status FROM smpp_messages WHERE id=?
sender.message.insert = INSERT INTO smpp_messages(smpp_status, status,  id) VALUES(?,?,?)
sender.message.update = UPDATE smpp_messages SET smpp_status=?, status=? WHERE id=?