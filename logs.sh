# Shell functions for logging messages.
# Prime Jyothi 20140124
# primejyothi [at] gmail [dot] com
function log ()
{
	lgLine=`printf "%4d" $1`
	shift
	echo "[Log   : ${lgLine}] $@"
}

function err ()
{
	lgLine=`printf "%4d" $1`
	shift
	echo "[Error : ${lgLine}] $@"
}

function dbg ()
{
	if [[ -z "$dbgFlag" ]]
	then
		return
	fi
	lgLine=`printf "%4d" $1`
	shift
	echo "[Debug : ${lgLine}] $@"
}
