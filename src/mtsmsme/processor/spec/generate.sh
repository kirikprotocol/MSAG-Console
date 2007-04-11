cd ../ncodec
rm -rf *.[ch]
#../spec/asn1c -fnative-types ../spec/ros.asn ../spec/dialogue.asn ../spec/mtsms.asn
../spec/asn1c -S /home/vic/projects/asn1c/skeletons -fnative-types ../spec/ros.asn ../spec/dialogue.asn ../spec/mtsms.asn
find . -type l  -exec rm {} \;
rm Makefile.am.sample