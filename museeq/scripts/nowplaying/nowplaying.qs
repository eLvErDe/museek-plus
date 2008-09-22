/*

 Fields:

  %(file)        filename or URL
                 [xmms, amarok, mpd]

  %(artist)      artist
                 [amarok, mp3blaster, mpd]
  %(track)       name of the track
                 [amarok, mp3blaster, mpd]
  %(title)       %(artist) - %(track)
                 [xmms, amarok, mp3blaster, mpd]
  %(album)       album the track is on
                 [amarok, mp3blaster, mpd]
  %(trackno)     track number on the album
                 [mpd]
  %(tracks)      tracks on the album
                 []
  %(year)        year the track was released
                 [amarok]
  %(genre)       the genre the track belongs to
                 [amarok]

  %(pos)         position in the current track (m:ss)
                 [xmms, amarok, mpd]
  %(pos_sec)     position in the current track(seconds)
                 [amarok]
  %(pos_usec)    position in the current track (useconds)
                 [xmms]
  %(pos_%)       position in the current track (percentage)
                 [mpd]

  %(length)      length of the current track (m:ss)
                 [xmms, amarok]
  %(length_sec)  length of the current track (seconds)
                 [amarok, mp3blaster]
  %(length_usec) length of the current track (useconds)
                 [xmms]

  %(bitrate)     bitrate
                 [xmms, amarok, mp3blaster]
  %(samplerate)  samplerate
                 [xmms, mp3blaster]
  %(channels)    channels of the media file (number)
                 [xmms]

  %(status)      status of the player
                 [xmms, mp3blaster, mpd]

  %(pl_length)   length of the playlist
                 [xmms, mpd]
  %(pl_current)  current position in playlist
                 [xmms, mpd]

*/

muscript.nowPlayingString = "/me is now listening to: %(title)";
muscript.nowPlayingInsertString = "%(title)";
muscript.nowPlayingCustomString = "/me is now listening to: %(title)";
muscript.nowPlayingPlayer = "xmms";
muscript.nowPlayingInfoPipe = "/tmp/xmms-info";
muscript.nowPlayingAmarok = "amarok";
muscript.nowPlayingMP3Blaster = "~/.mp3blaster-status";

muscript.nowPlayingExpandHome = function(p)
{
	if(p.substring(0, 1) == "~")
		return Dir.home + "/" + p.substring(1);
	else
		return p;
}

muscript.nowPlayingBuildXMMS = function(format)
{
	var title = "";
	var status = "";
	var tunes = "";
	var current = "";
	var usecpos = "";
	var pos = "";
	var bitrate = "";
	var samplefreq = "";
	var usectime = "";
	var time = "";
	var channels = "";
	var file = "";

	var infile = new File(this.nowPlayingExpandHome(this.nowPlayingInfoPipe));
	infile.open(File.ReadOnly);
	while (!infile.eof) {
		var line = infile.readLine();
		if(line == "")
			continue;
		line = line.substring(0, line.length - 1);
		if(line.startsWith("Title: "))
			title = line.substring(7);
		else if(line.startsWith("Status: "))
			status = line.substring(8);
		else if(line.startsWith("Tunes in playlist: "))
			tunes = line.substring(19);
		else if(line.startsWith("Currently playing: "))
			current = line.substring(19);
		else if(line.startsWith("uSecPosition: "))
			usecpos = line.substring(14);
		else if(line.startsWith("Position: "))
			pos = line.substring(10);
		else if(line.startsWith("uSecTime: "))
			usectime = line.substring(10);
		else if(line.startsWith("Time: "))
			time = line.substring(6);
		else if(line.startsWith("Current bitrate: "))
			bitrate = line.substring(17);
		else if(line.startsWith("Samping Frequency: "))
			samplefreq = line.substring(19);
		else if(line.startsWith("Sampling Frequency: "))
			samplefreq = line.substring(20);
		else if(line.startsWith("Channels: "))
			channels = line.substring(10);
		else if(line.startsWith("File: "))
			file = line.substring(6);
	}

	var s = format.replace("%(title)", title);
	s = s.replace("%(file)", file);
	s = s.replace("%(status)", status);
	s = s.replace("%(pl_length)", tunes);
	s = s.replace("%(pl_current)", current);
	s = s.replace("%(pos_usec)", usecpos);
	s = s.replace("%(pos)", pos);
	s = s.replace("%(length_usec)", usectime);
	s = s.replace("%(length)", time);
	s = s.replace("%(bitrate)", bitrate);
	s = s.replace("%(samplerate)", samplefreq);
	s = s.replace("%(channels)", channels);

	return s;
}

muscript.nowPlayingGetAmarok = function(field)
{
    return this.launchProcess([ 'dcop', this.nowPlayingAmarok, 'player', field ]);
}

muscript.nowPlayingBuildAmarok = function(format)
{
	var s = format;

	if(s.find("%(title)") >= 0)
		s = s.replace("%(title)", this.nowPlayingGetAmarok("nowPlaying"));

	if(s.find("%(pos)") >= 0)
		s = s.replace("%(pos)", this.nowPlayingGetAmarok("currentTime"));

	if(s.find("%(pos_sec") >= 0)
		s = s.replace("%(pos_sec)", this.nowPlayingGetAmarok("trackCurrentTime"));

	if(s.find("%(length)") >= 0)
		s = s.replace("%(length)", this.nowPlayingGetAmarok("totalTime"));

	if(s.find("%(length_sec") >= 0)
		s = s.replace("%(length_sec)", this.nowPlayingGetAmarok("trackTotalTime"));

	if(s.find("%(bitrate)") >= 0)
		s = s.replace("%(bitrate)", this.nowPlayingGetAmarok("bitrate"));

	if(s.find("%(album)") >= 0)
		s = s.replace("%(album)", this.nowPlayingGetAmarok("album"));

	if(s.find("%(file)") >= 0)
		s = s.replace("%(file)", this.nowPlayingGetAmarok("encodedURL"));

	if(s.find("%(year)") >= 0)
		s = s.replace("%(year)", this.nowPlayingGetAmarok("year"));

	if(s.find("%(genre)") >= 0)
		s = s.replace("%(genre)", this.nowPlayingGetAmarok("genre"));

	if(s.find("%(status)") >= 0)
	{
		status = this.nowPlayingGetAmarok("status");
		if(status == "0")
			s = s.replace("%(status)", "stopped");
		else if(status == "1")
			s = s.replace("%(status)", "paused");
		else if(status == "2")
			s = s.replace("%(status)", "playing");
		else
			s = s.replace("%(status)", "unknown");
	}

	return s;
}

muscript.nowPlayingBuildMPD = function(format)
{
	format = format.replace ("%(artist)" , "%artist%");
	format = format.replace ("%(track)"  , "%title%");
	format = format.replace ("%(title)"  , "%artist% - %title%");
	format = format.replace ("%(album)"  , "%album%");
	format = format.replace ("%(trackno)", "%track%");
	format = format.replace ("%(file)"   , "%file%");

	var a = this.launchProcess([ 'mpc', '--format', format ]).split ("\n");
	var b = a[1].split (" ");
	var status = b[0].substring (1, b[0].length - 1);
	var c = b[1].substring (1, b[1].length).split ("/");
	var pl_current = c[0];
	var pl_length  = c[1];
	var pos_pctg   = b[5].substring (1, b[5].length - 1);
	var pos        = b[4];

	a = a[0].replace ("%(status)" , status);
	a = a.replace ("%(pos)"       , pos);
	a = a.replace ("%(pl_length)" , pl_length);
	a = a.replace ("%(pl_current)", pl_current);
	a = a.replace ("%(pos_%)"     , pos_pctg);

  	return a;
}

muscript.nowPlayingBuildMP3Blaster = function(format)
{
	var file = "";
	var status = "";
	var artist = "";
	var album = "";
	var title = "";
	var bitrate = "";
	var samplerate = "";
	var length = "";

	var infile = new File(this.nowPlayingExpandHome(this.nowPlayingMP3Blaster));
	infile.open(File.ReadOnly);
	while (!infile.eof) {
		var line = infile.readLine();
		if(line == "")
			continue;
		line = line.substring(0, line.length - 1);
		if(line.startsWith("path "))
			file = line.substring(5);
		else if(line.startsWith("status "))
			status = line.substring(7);
		else if(line.startsWith("title "))
			title = line.substring(6);
		else if(line.startsWith("bitrate "))
			bitrate = line.substring(8);
		else if(line.startsWith("samplerate "))
			samplerate = line.substring(11);
		else if(line.startsWith("artist "))
			artist = line.substring(7);
		else if(line.startsWith("album "))
			album = line.substring(6);
		else if(line.startsWith("length "))
			length = line.substring(7);
	}

	var s = format.replace("%(title)", artist + " - " + title);
	s = s.replace("%(artist)", artist);
	s = s.replace("%(album)", album);
	s = s.replace("%(track)", title);
	s = s.replace("%(file)", file);
	s = s.replace("%(status)", status);
	s = s.replace("%(bitrate)", bitrate);
	s = s.replace("%(length_sec)", length);
	return s.replace("%(samplerate)", samplerate);
}

muscript.nowPlayingBuild = function(format)
{
	if(this.nowPlayingPlayer == "xmms")
		return this.nowPlayingBuildXMMS(format);
	else if(this.nowPlayingPlayer == "amarok")
		return this.nowPlayingBuildAmarok(format);
	else if(this.nowPlayingPlayer == "mpd")
		return this.nowPlayingBuildMPD(format);
	else if(this.nowPlayingPlayer == "mp3blaster")
		return this.nowPlayingBuildMP3Blaster(format);

	return "";
}

muscript.nowPlayingCheck = function()
{
	if(! museeq.connected)
	{
		this.showWarning("You need to be connected to museekd to do this");
		return 0;
	}

	return 1;
}

muscript.nowPlaying = function()
{
	if(this.nowPlayingCheck() == 0)
		return;

	this.sayRoom(this.nowPlayingBuild(this.nowPlayingString));
}

muscript.nowPlayingInsert = function()
{
	if(this.nowPlayingCheck() == 0)
		return;

	var string = Input.getText("Text:", this.nowPlayingInsertString, "Insert now playing");
	if(string) {
		this.setConfig("insertString", string);
		museeq.mainWin.chatRooms.getCurrentWidget().panel.line.insert(this.nowPlayingBuild(string));
	}
}

muscript.nowPlayingCustom = function()
{
	if(nowPlayingCheck() == 0)
		return;

	var string = Input.getText("Text:", this.nowPlayingCustomString, "Send custom now playing text");
	if(string) {
		this.setConfig("customString", string);
		this.sayRoom(this.nowPlayingBuild(string));
	}
}

muscript.nowPlayingConfigChanged = function(domain, key, value)
{
	if(domain == "QSnowPlaying")
	{
		if(key == "string")
			this.nowPlayingString = value;
		else if(key == "insertString")
			this.nowPlayingInsertString = value;
		else if(key == "customString")
			this.nowPlayingCustomString = value;
		else if(key == "player")
			this.nowPlayingPlayer = value;
		else if(key == "infopipe")
			this.nowPlayingInfoPipe = value;
		else if(key == "amarok")
			this.nowPlayingAmarok = value;
		else if(key == "mp3blaster")
			this.nowPlayingMP3Blaster = value;
	}
}

muscript.nowPlayingConfigure = function()
{
	if(! museeq.connected)
	{
		this.showWarning("You need to be connected to museekd to do this");
		return;
	}

    this.ui = this.loadUI(this.path() + "/nowplaying_config.ui");

    var npText = this.ui.findChild("npText");
    var infopipe = this.ui.findChild("infopipePath");
    var amarokDcop = this.ui.findChild("amarokDcop");
    var mp3blasterPath = this.ui.findChild("mp3blasterPath");
    var xmms = this.ui.findChild("xmms");
    var amarok = this.ui.findChild("amarok");
    var mpd = this.ui.findChild("mpd");
    var mp3blaster = this.ui.findChild("mp3blaster");
    var buttonBox = this.ui.findChild("buttonBox");

    if (npText)
        npText.text = this.nowPlayingString;
    if (infopipe)
        infopipe.text = this.nowPlayingInfoPipe;
    if (amarokDcop)
        amarokDcop.text = this.nowPlayingAmarok;
    if (mp3blasterPath)
        mp3blasterPath.text = this.nowPlayingMP3Blaster;

	if(this.nowPlayingPlayer == "xmms" && xmms)
		xmms.checked = true;
	if(this.nowPlayingPlayer == "amarok" && amarok)
		amarok.checked = true;
	if(this.nowPlayingPlayer == "mpd" && mpd)
		mpd.checked = true;
	if(this.nowPlayingPlayer == "mp3blaster" && mp3blaster)
		mp3blaster.checked = true;

    if (buttonBox) {
        buttonBox.accepted.connect(this, this.nowPlayingConfigureValidated);
        buttonBox.rejected.connect(this, this.nowPlayingConfigureRejected);
    }

    this.ui.show();
}

muscript.nowPlayingConfigureValidated = function() {
    var npText = this.ui.findChild("npText");
    var infopipe = this.ui.findChild("infopipePath");
    var amarokDcop = this.ui.findChild("amarokDcop");
    var mp3blasterPath = this.ui.findChild("mp3blasterPath");
    var xmms = this.ui.findChild("xmms");
    var amarok = this.ui.findChild("amarok");
    var mpd = this.ui.findChild("mpd");
    var mp3blaster = this.ui.findChild("mp3blaster");
    var buttonBox = this.ui.findChild("buttonBox");

    if(xmms && xmms.checked)
        this.setConfig("player", "xmms");
    else if(amarok && amarok.checked)
        this.setConfig("player", "amarok");
    else if(mpd && mpd.checked)
        this.setConfig("player", "mpd");
    else
        this.setConfig("player", "mp3blaster");

    if (npText)
        this.setConfig("string", npText.text);
    if (infopipe)
        this.setConfig("infopipe", infopipe.text);
    if (amarokDcop)
        this.setConfig("amarok", amarokDcop.text);
    if (mp3blasterPath)
        this.setConfig("mp3blaster", mp3blasterPath.text);

    this.ui.close();
}

muscript.nowPlayingConfigureRejected = function() {
    this.ui.close();
}

muscript.nowPlayingInputHandler = function(privateMessage, target, line)
{
	if(line == "np")
		return this.nowPlayingBuild(nowPlayingString);
	else if(line.startsWith("np "))
		return this.nowPlayingBuild(line.substring(3));
}

muscript.init = function()
{
	var value = this.config("string");
	if(value)
		this.nowPlayingString = value;

	value = this.config("insertString");
	if(value)
		this.nowPlayingInsertString = value;

	value = this.config("customString");
	if(value)
		this.nowPlayingCustomString = value;

	value = this.config("player");
	if(value)
		this.nowPlayingPlayer = value;

	value = this.config("infopipe");
	if(value)
		this.nowPlayingInfoPipe = value;

	value = this.config("amarok");
	if(value)
		this.nowPlayingAmarok = value;

	value = this.config("mp3blaster");
	if(value)
		this.nowPlayingMP3Blaster = value;

    museeq.configChanged.connect(this.nowPlayingConfigChanged);
	this.addMenu("Scripts", "Send default now playing", "nowPlaying");
	this.addMenu("Scripts", "Send custom now playing", "nowPlayingCustom");
	this.addMenu("Scripts", "Insert now playing", "nowPlayingInsert");
	this.addMenu("Settings", "Configure now playing", "nowPlayingConfigure");

	this.addInputHandler("nowPlayingInputHandler");

	return "NowPlaying";
}
