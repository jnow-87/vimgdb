	#########################
	###                   ###
	###   global config   ###
	###                   ###
	#########################

QUIET ?= @
DEBUG ?= 0

supported_flags := cflags cxxflags cppflags asflags ldflags archflags yaccflags lexflags
supported_types := obj lib bin


# disable built-in rules
.SUFFIXES:

# disable removal of temporary files
.SECONDARY:


	####################
	###              ###
	###   commands   ###
	###              ###
	####################

CC ?= gcc
CXX ?= g++
AS ?= as
LD ?= ld
AR ?= ar
LEX ?= flex
YACC ?= bison
cc := $(CC)
cxx := $(CXX)
as := $(AS)
ld := $(LD)
ar := $(AR)
lex := $(LEX)
yacc := $(YACC)
echo := @echo -e
printf := @printf
rm := $(QUIET)rm -rf
mkdir := $(QUIET)mkdir -p
touch := $(QUIET)touch


	#################
	###           ###
	###   DEBUG   ###
	###           ###
	#################

# print DEBUG message
#
#	$(call pdebug,<msg>)
ifneq ($(DEBUG),0)
  define pdebug
    $(info $1)
  endef
else
  define pdebug
  endef
endif


	###############################
	###                         ###
	###   directory traversal   ###
	###                         ###
	###############################

build := scripts/Makefile.build


# recursively include 'Makefile.traverse'	on given directories
# directory for current iteration is available through $(loc_dir)
#
#	$(call dinclude,<directory list>)
define dinclude
	$(foreach d,$(1), \
		$(eval loc_dir=$(d)) \
		$(eval include $(build)) \
	)
endef


	###########################
	###                     ###
	###   file operations   ###
	###                     ###
	###########################

# return files that do not define a separate list of prerequisites,
# i.e. $(<file>-y) is empty
#
#	$(call filter_single_file,<file list>
define filter_single_prereq
	$(foreach f, $(1), \
		$(if $($(basename $(f))-y), \
			, \
			$(f) \
		) \
	)
endef

# return list of prerequisites for given list of files,
# i.e. return $(<file>-y)
#
#	$(call get_prereq,<file list>
define get_prereq
	$(foreach f,$(1), \
		$($(basename $(f))-y) \
	)
endef


	############################
	###                      ###
	###   rules generation   ###
	###                      ###
	############################

# generate target-specific rule for <target>-*flags
# 	afterwards clear flag variable to avoid name collisions with files
# 		with the same stem but in a different directory
#
#	$(call gen_rule_tgt_flags,<target>,<flag>,<bin_prefix>
define gen_rule_tgt_flags
	$(if $($(basename $(1))-$(2)), \
		$(eval $(call gen_rule_basic,,$(3)$(1), $(2) += $($(basename $(1))-$(2)))) \
		$(eval $(basename $(1))-$(2) :=), \
	)
endef

# generate target-specific rule for local flags, i.e. *flags-y and subdir-*flags
#
#	$(call gen_rule_flags,<target>,<flag>,<bin_prefix>
define gen_rule_loc_flags
	$(if $(strip $($(2)-y) $(subdir-$(2))), \
		$(eval $(call gen_rule_basic,,$(3)$(1), $(2) += $($(2)-y) $(subdir-$(2)))) \
		,)
endef

# generate rules for compound targets
# 	prerequisite list is appended with given <bin_prefix>
#	afterwards prerequisite variable is cleared to avoid name collisions
#		with files with the same stem but in a different diretory
#
#	$(call gen_rule_multi,<cmd-name>,<file list>,<bin_prefix>	
define gen_rule_multi
		$(foreach f,$(2), \
			$(call gen_rule_basic,$(1),$(3)$(f),$(addprefix $(3),$($(basename $(f))-y))) \
			$(eval $(basename $(f))-y :=) \
		)
endef

# generate basic rule
#
#	$(call gen_rule_basic,<cmd-name>,<target>,<prerequisite>
define gen_rule_basic
	$(eval \
		$(call pdebug,    generate rule:)
		$(call pdebug,        $(2): $(3))
		$(if $(1), \
			$(call pdebug,            $$(call $(1),$(host))) \
		)
		$(call pdebug)

		$(if $(1),
			$(eval \
				$(2): $(3)
					$$(call $(1),$(host)) \
			), \
			$(eval $(2): $(3)) \
		) \
	)
endef


	####################
	###              ###
	###   commands   ###
	###              ###
	####################

# XXX $(call cmd_*,<host>)

define cmd_cc_c_o 
	$(printf) "[$(1)C-O]   %60.60s %60.60s   -o %-40.40s\n" "$(cflags) | $(cppflags) | $(ldflags) | $(asflags) | $(archflags)" "$^" "$@"
	$(mkdir) $(@D)
	$(touch) $@
endef

# XXX use ld -r to combine objects
define cmd_cc_o_o 
	$(printf) "[$(1)O-O]   %60.60s %60.60s   -o %-40.40s\n" "$(cflags) | $(cppflags) | $(ldflags) | $(asflags) | $(archflags)" "$^" "$@"
	$(mkdir) $(@D)
	$(touch) $@
endef

define cmd_cc_cxx_o 
	$(printf) "[$(1)CXX-O] %60.60s %60.60s   -o %-40.40s\n" "$(cflags) | $(cxxflags) | $(cppflags) | $(ldflags) | $(asflags) | $(archflags)" "$^" "$@"
	$(mkdir) $(@D)
	$(touch) $@
endef

define cmd_cc_s_o
	$(printf) "[$(1)S-O]   %60.60s %60.60s   -o %-40.40s\n" "$(cflags) | $(cppflags) | $(ldflags) | $(asflags) | $(archflags)" "$^" "$@"
	$(mkdir) $(@D)
	$(touch) $@
endef

define cmd_cc_c_i 
	$(printf) "[$(1)C-I]   %60.60s %60.60s   -o %-40.40s\n" "$(cflags) | $(cppflags) | $(ldflags) | $(asflags) | $(archflags)" "$^" "$@"
	$(mkdir) $(@D)
	$(touch) $@
endef

define cmd_cc_s_i
	$(printf) "[$(1)S-I]   %60.60s %60.60s   -o %-40.40s\n" "$(cflags) | $(cppflags) | $(ldflags) | $(asflags) | $(archflags)" "$^" "$@"
	$(mkdir) $(@D)
	$(touch) $@
endef

define cmd_cc_y_c
	$(printf) "[Y-C]   %60.60s %60.60s   -o %-40.40s\n" "$(cflags) | $(cppflags) | $(ldflags) | $(asflags) | $(archflags) | $(yaccflags)" "$^" "$@"
	$(mkdir) $(@D)
	$(touch) $@
endef

define cmd_cc_l_c
	$(printf) "[L-C]   %60.60s %60.60s   -o %-40.40s\n" "$(cflags) | $(cppflags) | $(ldflags) | $(asflags) | $(archflags) | $(lexflags)" "$^" "$@"
	$(mkdir) $(@D)
	$(touch) $@
endef

define cmd_cc_o_lib
	$(printf) "[$(1)O-L]   %60.60s %60.60s   -o %-40.40s\n" "$(cflags) | $(cppflags) | $(ldflags) | $(asflags) | $(archflags)" "$^" "$@"
	$(mkdir) $(@D)
	$(touch) $@
endef

define cmd_cc_o_bin
	$(printf) "[$(1)O-B]   %60.60s %60.60s   -o %-40.40s\n" "$(cflags) | $(cppflags) | $(ldflags) | $(asflags) | $(archflags)" "$^" "$@"
	$(mkdir) $(@D)
	$(touch) $@
endef

define cmd_cc_c_lib
	$(printf) "[$(1)C-L]   %60.60s %60.60s   -o %-40.40s\n" "$(cflags) | $(cppflags) | $(ldflags) | $(asflags) | $(archflags)" "$^" "$@"
	$(mkdir) $(@D)
	$(touch) $@
endef

define cmd_cc_c_bin
	$(printf) "[$(1)C-B]   %60.60s %60.60s   -o %-40.40s\n" "$(cflags) | $(cppflags) | $(ldflags) | $(asflags) | $(archflags)" "$^" "$@"
	$(mkdir) $(@D)
	$(touch) $@
endef
