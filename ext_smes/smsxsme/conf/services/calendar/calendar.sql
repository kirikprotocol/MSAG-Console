# Calendar Message has following fields
# id int not null                            - unique identificator
# source_address varchar(30) not null        - source abonent address
# destination_address varshar(30) not null   - destination abonent address
# send_time timestamp not null default now() - send message time
# dest_addr_subunit int not null             - Destination address subunit: 1 - flush, 0 - usual, may be other values
# status int not null                        - status of message: 0 - new, 1 - processed
# smpp_status                                - smpp return status
# save_delivery_status int not null          - delivery status save flag: 1- save, 0 - not save
# message text not null                      - message text

calendar.message.load.max.id = SELECT max(id) FROM calendar_messages;

calendar.message.load.by.date = SELECT id, source_address, destination_address, send_time, dest_addr_subunit, message, status, save_delivery_status, smpp_status, conn FROM calendar_messages WHERE send_time < ? AND status=0 ORDER BY send_time LIMIT 0, ?
calendar.message.load.by.id = SELECT id, source_address, destination_address, send_time, dest_addr_subunit, message, status, save_delivery_status, smpp_status, conn FROM calendar_messages WHERE id=?
calendar.message.remove.by.id = DELETE FROM calendar_messages WHERE id=?
calendar.message.insert = INSERT INTO calendar_messages(source_address, destination_address, send_time, dest_addr_subunit, message, status, save_delivery_status, smpp_status, conn, id) VALUES(?,?,?,?,?,?,?,?,?,?)
calendar.message.update = UPDATE calendar_messages set source_address=?, destination_address=?, send_time=?, dest_addr_subunit=?, message=?, status=?, save_delivery_status=?, smpp_status=?, conn=? WHERE id=?