 #!/bin/bash
uninstall_file()
{
	cat version.txt
	rm /etc/remotes/ -rf
	rm /usr/sbin/remote-service -rf
	rm /lib/systemd/system/ecos_remote.service -rf
	rm /var/log/ecos_remote.log -rf
}
main()
{
	systemctl stop ecos_remote.service
	systemctl disable ecos_remote.service
	uninstall_file
	systemctl daemon-reload
}
main
