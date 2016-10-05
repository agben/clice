# bash completion for clice (CE) commands
# e.g.
#     edc ut_d[tab] to move to ~/Code/UT/ and complete from files starting ut_d

_ce_edit()
{
	local cur prev words cword
	_init_completion || return

#TODO - should take up to the 1st underscore for the directory name or complete up to it?
#		e.g. edc u[tab] could expand to edc ut_ if UT were the only matching source directory

	if [[  ${#cur} > 1 ]]
	 then
#TODO replace my directory with a clice config home folder
		SOURCEDIR="/home/ben/Code/$(echo ${cur:0:2}|tr [a-z] [A-Z])"
		if [ -d "$SOURCEDIR" ]				# does project directory exist?
		 then
			cd ${SOURCEDIR}/
			COMPREPLY=( $( compgen -f ${cur} ) )
		fi
	fi

	return 0
}
complete -F _ce_edit edc
