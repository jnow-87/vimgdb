	#########################
	###                   ###
	###   global config   ###
	###                   ###
	#########################

QUIET ?= @
SILENT ?= @
DEBUG ?= 0

ifdef SILENT
override QUIET := @
override SILENT := @
endif

ifdef QUIET
override QUIET := @
endif

supported_flags := cflags cxxflags cppflags asflags ldflags archflags hostcflags hostcxxflags hostcppflags hostasflags hostldflags hostarchflags yaccflags lexflags gperfflags
supported_types := obj obj-nobuiltin lib bin


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
HOSTCC ?= gcc
HOSTCXX ?= g++
HOSTAS ?= as
HOSTLD ?= ld
HOSTAR ?= ar
LEX ?= flex
YACC ?= bison
GPERF ?= gperf
cc := $(SILENT)$(CC)
cxx := $(SILENT)$(CXX)
as := $(SILENT)$(AS)
ld := $(SILENT)$(LD)
ar := $(SILENT)$(AR)
hostcc := $(SILENT)$(HOSTCC)
hostcxx := $(SILENT)$(HOSTCXX)
hostas := $(SILENT)$(HOSTAS)
hostld := $(SILENT)$(HOSTLD)
hostar := $(SILENT)$(HOSTAR)
lex := $(SILENT)$(LEX)
yacc := $(SILENT)$(YACC)
gperf := $(SILENT)$(GPERF)
echo := @echo
printf := @printf
rm := $(QUIET)rm -rf
mkdir := $(QUIET)mkdir -p
touch := $(QUIET)touch
cp := $(QUIET)cp

define exists
$(if $(wildcard $(1)),1,)
endef


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
included :=


# recursively include 'Makefile.build' on given directories, avoiding
# dobble-inclusion of the same Makefile
# directory for current iteration is available through $(loc_dir)
#
#	$(call dinclude,<directory list>)
define dinclude
	$(foreach d,$(filter-out $(included),$(1)), \
		$(eval loc_dir=$(d)) \
		$(eval included += $(d)) \
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
# 	first $(<target>-y), which is $($(basename $(f))-y) is checked for external prerequisites ($(edep)), i.e. prerequisites that
# 		origniate from a different path, are extracted
#
#	afterwards the rule is generated whereat <bin-prefix> is added to all non-external prerequisites and external prerequisites are
#		added as they are
#
#	finally the prerequisite variable is cleared to avoid name collisions
#		with files with the same stem but in a different diretory
#
#	$(call gen_rule_multi,<cmd-name>,<file list>,<bin_prefix>
define gen_rule_multi
		$(foreach f,$(2), \
			$(eval edep :=) \
			$(foreach p,$($(basename $(f))-y), \
				$(if $(subst ./,,$(dir $(p))), \
					$(eval edep += $(p)) \
					, \
				) \
			) \
			$(call gen_rule_basic,$(1),$(3)$(f),$(addprefix $(3),$(filter-out $(edep),$($(basename $(f))-y))) $(edep)) \
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
			$(call pdebug,            $(mkdir) $$(@D)) \
			$(call pdebug,            $$(call $(1),$(host))) \
		)
		$(call pdebug)

		$(if $(1),
			$(eval \
				$(2): $(3)
					$(mkdir) $$(@D)
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

# XXX naming: cmd_cc_<target type>_<prerequisite type>
# XXX $(call cmd_*,<host>)

define cmd_cc_i
	$(echo) [$(1)CC] $@
	$($(1)cc) $($(1)cppflags) $($(1)archflags) -E $< -o $@
	$($(1)cc) $($(1)cppflags) $($(1)archflags) -MM -MF $@.d -MP -MT $@ $<
endef

define cmd_cc_c_y
	$(echo) [YACC] $@
	$(yacc) $(yaccflags) --defines=$(basename $@).h $< -o $@
endef

define cmd_cc_c_l
	$(echo) [LEX] $@
	$(lex) $(lexflags) --header-file=$(basename $@).h -o $@ $<
endef

define cmd_cc_c_h
	$(echo) [GPERF] $@
	$(gperf) $(gperfflags) $< --output-file=$@
endef

define cmd_cc_o_s
	$(echo) [$(1)AS] $@
	$($(1)as) $($(1)asflags) $^ -o $@
endef

define cmd_cc_o_c
	$(echo) [$(1)CC] $@
	$($(1)cc) $($(1)cflags) $($(1)cppflags) $($(1)archflags) -c $< -o $@
	$($(1)cc) $($(1)cppflags) $($(1)archflags) -MM -MF $@.d -MP -MT $@ $<
endef

define cmd_cc_o_cxx
	$(echo) [$(1)CXX] $@
	$($(1)cxx) $($(1)cxxflags) $($(1)cflags) $($(1)cppflags) $($(1)archflags) -c $< -o $@
	$($(1)cxx) $($(1)cppflags) $($(1)archflags) -MM -MF $@.d -MP -MT $@ $<
endef

define cmd_cc_o_o
	$(echo) [$(1)LD] $@
	$($(1)ld) -r $^ -o $@
endef

define cmd_cc_lib_o
	$(echo) [$(1)AR] $@
	$($(1)ar) crs $@ $^
endef

define cmd_cc_bin_o
	$(echo) [$(1)CC] $@
	$($(1)cc) $($(1)archflags) $^ -o $@ $($(1)ldflags)
endef
