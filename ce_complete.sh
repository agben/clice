# bash completion for clice (CE) commands
# e.g.
#     edc ut_d[tab] to move to ~/Code/UT/ and complete from files starting ut_d

_ce_edit()
{
	local cur prev words cword
	_init_completion || return

#TODO replace hard-coding my directory with a clice config home folder

	if [[  ${#cur} -lt 2 ]]
	 then
		cd "/home/ben/Code/"
		COMPREPLY=( $( compgen -d -X '!??') )		# list all two character directories
	else
		SOURCEDIR="/home/ben/Code/$(echo ${cur:0:2}|tr [a-z] [A-Z])"
		if [ -d "$SOURCEDIR" ]						# does project directory exist?
		 then
			cd ${SOURCEDIR}/
			COMPREPLY=( $( compgen -f -X '!*.*' ${cur} ) )	# list files that match what's been typed (and have a file extension)
		fi
	fi

	return 0
}
complete -F _ce_edit edc


_ce_clice()
{
	local cur prev words cword
	_init_completion || return


	local opts="--help --version"					# list valid command options that match what's been typed
	COMPREPLY=( $( compgen -W "$opts" "$cur" ) )

	return 0
}
complete -F _ce_clice clice
