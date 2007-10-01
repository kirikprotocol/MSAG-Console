distribution.info.load.by.abonent = SELECT abonent, today_cnt, cur_cnt, cnt FROM sponsored_distribution_info WHERE abonent=?
distribution.info.insert = INSERT INTO sponsored_distribution_info(abonent,tm, today_cnt, cur_cnt, cnt) values(?,?,?,?,?)
distribution.info.update = UPDATE sponsored_distribution_info SET tm=?,today_cnt=?,cnt=? WHERE abonent=?

delivery.stats.insert = INSERT INTO sponsored_delivery_stats (address, date, cnt) VALUES (?,CURDATE(),1)
delivery.stats.update = UPDATE sponsored_delivery_stats set cnt=cnt+1 WHERE address=? AND date=CURDATE()

get.abonents = select abonent, cur_cnt from sponsored_distribution_info where today_cnt = ? and cur_cnt > 0 limit ?,?
get.total.abonents.count = select count(*) from sponsored_distribution_info where today_cnt=? and cur_cnt > 0
get.total.messages.count = select sum(cur_cnt) from sponsored_distribution_info where today_cnt=?
decrease.messages.count = update sponsored_distribution_info set cur_cnt=cur_cnt-1 where abonent=?