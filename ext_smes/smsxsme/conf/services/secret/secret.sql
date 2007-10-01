
# Secret user
# address varchar(30) not null - user address
# password text not null      - user password

secret.user.load.by.address = SELECT address, password FROM sec_users WHERE address=?
secret.user.load.by.addresses = SELECT address, password FROM sec_users WHERE address IN (0)
secret.user.insert = INSERT INTO sec_users (address, password) VALUES (?,?)
secret.user.update = UPDATE sec_users SET password=? WHERE address=?
secret.user.remove = DELETE FROM sec_users WHERE address=?


# Secret message
# id int not null                            - unique message idetfificator
# destination_address varchar(30) not null   - destination abonent address
# message text not null                      - message text
# source_address varchar(30) not null        - source abonent address
# send_date timestamp not null default now() - time when message arrive into server
# dest_addr_subunit int not null             - destination address subunit
# status int not null                        - process status: 0 - NEW, 1 - PROCESSED
# smpp_status int                            - smpp status
# save_delivery_status int not null          - delivery status save flag: 1- save, 0 - not save

secret.message.load.max.id = SELECT max(id) FROM sec_messages

secret.message.load.by.user = SELECT id, destination_address, message, source_address, send_date, dest_addr_subunit, status, save_delivery_status, smpp_status, notify_originator FROM sec_messages WHERE status=0 AND destination_address=? ORDER BY send_date ASC
secret.message.load.count.by.user = SELECT count(*) FROM sec_messages WHERE status=0 AND destination_address=?
secret.message.load.by.id= SELECT id, destination_address, message, source_address, send_date, dest_addr_subunit, status, save_delivery_status, smpp_status, notify_originator FROM sec_messages WHERE id=?
secret.message.insert = INSERT INTO sec_messages(destination_address, source_address, message, dest_addr_subunit, status, save_delivery_status, smpp_status, notify_originator, id) VALUES(?,?,?,?,?,?,?,?,?)
secret.message.update = UPDATE sec_messages SET destination_address=?, source_address=?, message=?, dest_addr_subunit=?, status=?, save_delivery_status=?, smpp_status=?, notify_originator=? WHERE id=?
secret.message.remove = DELETE FROM sec_messages WHERE id=?


# Common requests
secret.load.user.and.messages = SELECT t1.password, t2.id, t2.destination_address, t2.message, t2.source_address, t2.send_date, t2.dest_addr_subunit, t2.status, t2.save_delivery_status, t2.smpp_status, t2.notify_originator FROM sec_users t1	LEFT JOIN sec_messages t2 ON (t1.address = t2.destination_address && t2.status=0) WHERE t1.address=?;