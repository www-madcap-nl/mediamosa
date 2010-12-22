#
# Regular cron job for the mediamosa package
#
* * * * * www-data [ -x /etc/mediamosa/crontab ] && /etc/mediamosa/crontab
