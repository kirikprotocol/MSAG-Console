cd ../ulcodec
rm -rf *.[ch]
#../spec/asn1c -fnative-types ../spec/ros.asn ../spec/dialogue.asn ../spec/mtsms.asn
#../spec/asn1c
#/home/gvroman/bin/asn1c-0.9.19/asn1c/asn1c 
../spec/asn1c -pdu=all -S /home/vic/projects/asn1c/skeletons -fnative-types ../spec/ros.asn ../spec/map_ms_types.asn ../spec/map_cmn_types.asn ../spec/map_ext_types.asn ../spec/map_bs_code.asn ../spec/map_ts_code.asn ../spec/map_err_types.asn ../spec/ul.asn
find . -type l  -exec rm {} \;
rm Makefile.am.sample
rm pdu_collection.c