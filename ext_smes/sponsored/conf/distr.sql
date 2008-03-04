delivery.save = REPLACE INTO deliveries (subscriber_address, distribution_name, sended, total, start_date, end_date, send_date, timezone) VALUES (?,?,?,?,?,?,?,?)
delivery.update = UPDATE deliveries SET sended=?, total=?, start_date=?, end_date=?, send_date=?, timezone=? WHERE subscriber_address=? AND distribution_name=?
delivery.lookupactive = SELECT subscriber_address, distribution_name, sended, total, start_date, end_date, send_date, timezone FROM deliveries WHERE end_date > now() AND sended < total AND send_date < ? LIMIT 0,?
delivery.count = SELECT count(*) FROM deliveries WHERE end_date > ? AND timezone=? AND distribution_name=?

delivery.lookupactive1 = SELECT subscriber_address, distribution_name, sended, total, start_date, end_date, send_date, timezone, (total - 1) * (UNIX_TIMESTAMP(?) - UNIX_TIMESTAMP(start_date)) / (UNIX_TIMESTAMP(end_date) - UNIX_TIMESTAMP(start_date)) t1, (total - 1) * (UNIX_TIMESTAMP(?) - UNIX_TIMESTAMP(start_date)) / (UNIX_TIMESTAMP(end_date) - UNIX_TIMESTAMP(start_date)) t2 from deliveries HAVING (ceil(t1)>=0 OR t2 > 0) AND ceil(t1) < total AND ceil(t1) < t2 ORDER by t1 DESC; 

deliverystat.save = INSERT INTO delivery_stats(subscriber_address, date, delivered) VALUES(?,?,?)
deliverystat.increase = UPDATE delivery_stats SET delivered=delivered + ? WHERE subscriber_address=? AND date=?
deliverystat.aggregate = SELECT subscriber_address, sum(delivered) FROM delivery_stats WHERE date >= ? AND date < ? GROUP BY subscriber_address