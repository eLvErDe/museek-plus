function uptime()
{
	if(mainWin.chatRooms.current)
	{
		Process.execute('hostname', '');
		var hostname = Process.stdout.substring(0, Process.stdout.length - 1);
		
		Process.execute('uptime', '');
		sayRoom(mainWin.chatRooms.current, hostname + ' has been up for ' + Process.stdout.substring(0, Process.stdout.length - 1));
	}
}

function init()
{
	addMenu("Scripts", "Uptime", "uptime");
	return "Uptime";
}
