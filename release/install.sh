 #!/bin/bash


install_file()
{
	cat version.txt
	mkdir -p /usr/sbin/
	mkdir -p /etc/remotes
	mkdir -p /lib/systemd/system
	
	cp remote-service /usr/sbin/
	cp remote.config  /etc/remotes
	cp version.txt /etc/remotes
	cp ecos_remote.service /lib/systemd/system
}


main()
{
	sudo systemctl stop ecos_remote.service
	install_file
	systemctl daemon-reload
	echo "install successful"
	systemctl enable ecos_remote.service
	systemctl start ecos_remote.service
	echo "start up the ecos remote  service"
}

main
