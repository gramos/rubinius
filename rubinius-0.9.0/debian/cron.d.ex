#
# Regular cron jobs for the rubinius package
#
0 4	* * *	root	[ -x /usr/bin/rubinius_maintenance ] && /usr/bin/rubinius_maintenance
