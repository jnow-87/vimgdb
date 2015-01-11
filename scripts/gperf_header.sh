#!/bin/bash

#
#	generate header file based on gerf c++ output
#		header will contain class definition extracted
#		from c++ source
#

cfile=$1	# c++ source file
header=$2	# output file name


suffix=${cfile##*.}

if [[ "${suffix}" != "cc" && "${suffix}" != "cpp" ]];then
	echo -e $0: input file \"${cfile}\" is no C++ source file
	exit 1
fi

# generate macro based on header file
macro=$(basename ${header} | tr a-z. A-Z_)


# print header
echo -e "#ifndef "${macro} > ${header}
echo -e "#define "${macro}"\n\n" >> ${header}

# extract macros
grep -e '#define' ${cfile} >> ${header}
echo >> ${header}

# extract wordlist definition
sed -e '/wordlist\[\]/,/};/!d' ${cfile} >> ${header}
echo >> ${header}

# extract class
sed -e '/class/,/}/!d' ${cfile} >> ${header}
echo >> ${header}

# print footer
echo -e "\n#endif // "${macro} >> ${header}

exit 0
