delivery.save = REPLACE INTO deliveries (subscriber_address, distribution_name, sended, total, start_date, end_date, send_date, timezone) VALUES (?,?,?,?,?,?,?,?)
delivery.update = UPDATE deliveries SET sended=?, total=?, start_date=?, end_date=?, send_date=?, timezone=? WHERE subscriber_address=? AND distribution_name=?
delivery.lookupactive = SELECT subscriber_address, distribution_name, sended, total, start_date, end_date, send_date, timezone FROM deliveries WHERE end_date > now() AND sended < total AND send_date < ? LIMIT 0,?
delivery.count = SELECT count(*) FROM deliveries WHERE end_date > ? AND timezone=?

deliverystat.save = INSERT INTO delivery_stats(subscriber_address, date, delivered) VALUES(?,?,?)
deliverystat.increase = UPDATE delivery_stats SET delivered=delivered + ? WHERE subscriber_address=? AND date=?
deliverystat.aggregate = SELECT subscriber_address, sum(delivered) FROM delivery_stats WHERE date >= ? AND date < ? GROUP BY subscriber_address