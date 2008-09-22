muscript.uptime = function()
{
    var hostname = this.launchProcess('hostname');
    var uptime = this.launchProcess('uptime');

    this.sayRoom(hostname + ' has been up for ' + uptime);
}

muscript.init = function()
{
	this.addMenu("Scripts", "Uptime", "uptime");
	return "Uptime";
}
