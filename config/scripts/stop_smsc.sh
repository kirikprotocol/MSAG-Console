#! /bin/bash
touch stop_smsc_flag
kill `ps -efa|grep $USER|grep ./smsc|awk -e '{print \$2}'`
