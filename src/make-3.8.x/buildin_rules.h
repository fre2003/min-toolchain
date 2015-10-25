#ifndef __BUILTIN_RULES_H__
#define __BUILTIN_RULES_H__


typedef enum
{
  EN_MAKE_UNKNOWN,
  EN_MAKE_INC_CC_DEP,
  EN_MAKE_NOINC_CC,
  EN_MAKE_FILE_LIST,
  EN_MAKE_MODULE_LIST,
  EN_MAKE_TARGET
}MAKE_FEATURE_TYPE_E;

//////////////////////////////////////////////////////////////////////////
#define CHECK_BR_TYPE(chk)                  (chk == EN_MAKE_INC_CC_DEP || chk == EN_MAKE_NOINC_CC || chk == EN_MAKE_FILE_LIST || chk==EN_MAKE_MODULE_LIST)
#define CHKEC_LINES(p, f, len)              ((len == sizeof(f)) || len == sizeof(f) + 1) && (strncmp(p, f, sizeof(f) - 1) == 0)

#define CHECK_LINE_FEATURES(p, f, len, chk) if (CHKEC_LINES(p, f, len))  {return chk;} 

#define GET_L_F_LEN(chk, len)         if(chk == EN_MAKE_INC_CC_DEP)\
                                      {\
                                        len = sizeof(MAKE_IMPLICIT_CC_RULES_LINES) - 1;\
                                      }\
                                      else if(chk == EN_MAKE_NOINC_CC)\
                                      {\
                                        len = sizeof(MAKE_IMPLICIT_FORCE_CC_RULES_LINES) - 1;\
                                      }\
                                      else if(chk == EN_MAKE_FILE_LIST)\
                                      {\
                                        len = sizeof(MAKE_FILE_LIST_MAKE_LINES) - 1;\
                                      }\
                                      else if(chk == EN_MAKE_MODULE_LIST)\
                                      {\
                                        len = sizeof(MAKE_MODULE_LIST_LINES) - 1;\
                                      }

#define CPY_L_F_LINES(chk, p)         if(chk == EN_MAKE_INC_CC_DEP)\
                                      {\
                                        strcpy(p, MAKE_IMPLICIT_CC_RULES_LINES);\
                                      }\
                                      else if(chk == EN_MAKE_NOINC_CC)\
                                      {\
                                        strcpy(p, MAKE_IMPLICIT_FORCE_CC_RULES_LINES);\
                                      }\
                                      else if(chk == EN_MAKE_FILE_LIST)\
                                      {\
                                        strcpy(p, MAKE_FILE_LIST_MAKE_LINES);\
                                      }\
                                      else if(chk == EN_MAKE_MODULE_LIST)\
                                      {\
                                        strcpy(p, MAKE_MODULE_LIST_LINES);\
                                      }
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
#define MAKE_DEFAULT_FEATURES                     ".M"
#define MAKE_IMPLICIT_CC_RULES_FEATURES           ".MAKE_IMPLICIT_CC_RULES"  // increment compile
#define MAKE_IMPLICIT_FORCE_CC_RULES_FEATURES     ".MAKE_IMPLICIT_FORCE_CC_RULES" // no increment compile
#define MAKE_FILE_LIST_MAKE_FEATURES              ".MAKE_FILE_LIST_MAKE_FEATURES"
#define MAKE_MODULE_LIST_TARGETS_FEATURES         ".MAKE_MODULE_LIST_TARGETS"

#define MAKE_XXX_FEATURES_MIN   MAKE_IMPLICIT_CC_RULES_FEATURES
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// increament compile with depency files(.d)
// 
// MAKE_DEFAULT_NOINC_CC_FEATURES
//
// MAKE_C_SRC_FILES     like: c:/dir/1.c c:/dir/2.c
// MAKE_S_SRC_FILES
// MAKE_CPP_SRC_FILES
// MAKE_XX_SRC_FILES 
// CC_XX CC_XX_OPTION
// define target var : MAKE_IMPLICIT_CC_RULES_TARGET
//////////////////////////////////////////////////////////////////////////
#define MAKE_IMPLICIT_CC_RULES_LINES \
"MAKE_CC_RC_OBJ_EXT_NAME ?=res\n"\
"MAKE_CC_RC_SRC_EXT_NAME ?=rc\n"\
"MAKE_CC_XX_SRS_EXT_NAME ?=xxc\n"\
"MAKE_CC_XX_OBJ_EXT_NAME ?=xxo\n"\
"ifdef MAKE_DEFAULT_MSVC_FEATURES_SWT\n"\
"MAKE_CC_OBJ_EXT_NAME ?=obj\n"\
"MAKE_INC_PATH_OPTION_PREFIX ?=/I\n"\
"MAKE_CMD_OPTION_PREFIX ?=/\n"\
"else\n"\
"MAKE_CC_OBJ_EXT_NAME ?=o\n"\
"MAKE_INC_PATH_OPTION_PREFIX ?=-I\n"\
"MAKE_CMD_OPTION_PREFIX ?=-\n"\
"endif\n"\
"_MAKE_FULL_INC_PATH_OPTION =$(strip $(MAKE_FULL_INC_PATH_OPTION))\n"\
"_MAKE_OBJ_OUTPUT_PATH =$(strip $(MAKE_OBJ_OUTPUT_PATH))\n"\
"\n"\
"ifdef MAKE_IMPLICIT_CC_RULES_TARGET\n" \
"ifneq ($(strip $(MAKE_STD_SRC_FILE)),)\n"\
"MAKE_STD_OBJ_FILE ?= $(_MAKE_OBJ_OUTPUT_PATH)/$(basename $(notdir $(MAKE_STD_SRC_FILE))).$(MAKE_CC_OBJ_EXT_NAME)\n"\
"endif\n"\
"MAKE_FULL_SRC_RES_FILES ?= ${foreach obj,${MAKE_RC_SRC_FILES},$(_MAKE_OBJ_OUTPUT_PATH)/$(basename $(notdir $(obj))).$(MAKE_CC_RC_OBJ_EXT_NAME)}\n"\
"MAKE_FULL_SRC_XX_FILES ?= ${foreach obj,${MAKE_XX_SRC_FILES},$(_MAKE_OBJ_OUTPUT_PATH)/$(basename $(notdir $(obj))).$(MAKE_CC_XX_OBJ_EXT_NAME)}\n"\
"MAKE_FULL_SRC_OBJ_FILES ?= ${foreach obj,$(MAKE_S_SRC_FILES) ${MAKE_C_SRC_FILES} ${MAKE_CPP_SRC_FILES} ${MAKE_XX_SRC_FILES},$(_MAKE_OBJ_OUTPUT_PATH)/$(basename $(notdir $(obj))).$(MAKE_CC_OBJ_EXT_NAME)}\n"\
"endif # MAKE_IMPLICIT_CC_RULES_TARGET\n"\
\
"\n"\
\
"ifneq ($(strip $(MAKE_DISABLE_GCC_DEP_FEATURE)),YES)\n"\
"ifneq ($(MAKECMDGOALS),clean)\n"\
\
"ifdef MAKE_DEFAULT_MSVC_FEATURES_SWT\n"\
"MAKE_DEP_CMD_LINE_OPT ?=-e -w -lang-ms\n"\
"else\n"\
"MAKE_DEP_CMD_LINE_OPT ?=-e -w\n"\
"endif\n"\
"MAKE_DEP_CMD ?=@gdep\n"\
"_MAKE_PATH_TMP_INC = $(subst $(MAKE_INC_PATH_OPTION_PREFIX),,$(subst $(strip $(MAKE_INC_PATH_OPTION_PREFIX)),,$(strip $(MAKE_FULL_INC_PATH_OPTION))))\n"\
"_MAKE_PATH_TMP_INC_TF = $(filter-out \"$(strip $(MAKE_DEP_EXCLUDE_INC_PATH))\",$(filter-out $(strip $(MAKE_DEP_EXCLUDE_INC_PATH)),$(_MAKE_PATH_TMP_INC)))\n"\
"MAKE_DEP_INC_PATH_OPTION ?= $(addprefix $(MAKE_INC_PATH_OPTION_PREFIX), $(_MAKE_PATH_TMP_INC_TF))\n"\
"MAKE_DEP_CMD_LINE ?= $(MAKE_DEP_CMD) -I$(basedir $<) $(subst /I,-I,$(MAKE_DEP_INC_PATH_OPTION)) $< $(MAKE_DEP_CMD_LINE_OPT) -MM -dF -o $@ -objpath $(_MAKE_OBJ_OUTPUT_PATH)\n"\
"\n"\
\
"ifneq ($(strip $(MAKE_STD_OBJ_FILE)),)\n"\
"-include $(MAKE_STD_OBJ_FILE:.$(MAKE_CC_OBJ_EXT_NAME)=.d)\n"\
"endif\n"\
"-include $(MAKE_FULL_SRC_OBJ_FILES:.$(MAKE_CC_OBJ_EXT_NAME)=.d)\n"\
"ifdef MAKE_CC_RC_OBJ_EXT_NAME\n"\
"-include $(MAKE_FULL_SRC_RES_FILES:.$(MAKE_CC_RC_OBJ_EXT_NAME)=.d)\n"\
"endif\n"\
\
"endif # clean\n"\
"endif # MAKE_DISABLE_GCC_DEP_FEATURE\n"\
"\n"\
\
"ifdef MAKE_IMPLICIT_CC_RULES_TARGET\n" \
"$(MAKE_IMPLICIT_CC_RULES_TARGET) : $(MAKE_FULL_SRC_OBJ_PRE) $(MAKE_STD_OBJ_FILE) $(MAKE_FULL_SRC_OBJ_FILES) $(MAKE_FULL_SRC_RES_FILES) $(MAKE_FULL_USER_OBJ_FILES) $(MAKE_FULL_SRC_OBJ_POST)\n"\
"\n"\
\
"MAKE_CC_SRC_FILE_ECHO ?=@print Compile $<\n"\
"MAKE_XX_SRC_FILE_ECHO ?=@print Compile $<\n"\
\
"MAKE_CMD_S_C ?=$(MAKE_CMD_OPTION_PREFIX)c\n"\
"MAKE_CMD_S_INC_PATH_OPTION ?=$(_MAKE_FULL_INC_PATH_OPTION)\n"\
"MAKE_CMD_XX_C ?=$(MAKE_CMD_OPTION_PREFIX)c\n"\
"MAKE_CMD_XX_INC_PATH_OPTION ?=$(_MAKE_FULL_INC_PATH_OPTION)\n"\
\
"MAKE_CC_STD ?= $(strip $(CC_STD)) $(strip $(CC_STD_OPTION)) $(MAKE_INC_PATH_OPTION_PREFIX)$(basedir $<) $(_MAKE_FULL_INC_PATH_OPTION) $(MAKE_CMD_OPTION_PREFIX)c $< $(MAKE_CMD_OPTION_OUTPUT) $(_MAKE_OBJ_OUTPUT_PATH)/$(notdir $@)\n"\
"MAKE_CC_C ?= $(strip $(CC)) $(strip $(CC_OPTION)) $(MAKE_INC_PATH_OPTION_PREFIX)$(basedir $<) $(_MAKE_FULL_INC_PATH_OPTION) $(MAKE_CMD_OPTION_PREFIX)c $< $(MAKE_CMD_OPTION_OUTPUT) $(_MAKE_OBJ_OUTPUT_PATH)/$(notdir $@)\n"\
"MAKE_CC_S ?= $(strip $(CC_ASM)) $(strip $(CC_ASM_OPTION)) $(MAKE_INC_PATH_OPTION_PREFIX)$(basedir $<) $(MAKE_CMD_S_INC_PATH_OPTION) $(MAKE_CMD_S_C) $< $(MAKE_CMD_OPTION_OUTPUT) $(_MAKE_OBJ_OUTPUT_PATH)/$(notdir $@)\n"\
"MAKE_CC_CPP ?= $(strip $(CC_CPP)) $(strip $(CC_CPP_OPTION)) $(MAKE_INC_PATH_OPTION_PREFIX)$(basedir $<) $(_MAKE_FULL_INC_PATH_OPTION) $(MAKE_CMD_OPTION_PREFIX)c $< $(MAKE_CMD_OPTION_OUTPUT) $(_MAKE_OBJ_OUTPUT_PATH)/$(notdir $@)\n"\
"MAKE_CC_XX ?= $(strip $(CC_XX)) $(strip $(CC_XX_OPTION)) $(MAKE_INC_PATH_OPTION_PREFIX)$(basedir $<) $(MAKE_CMD_XX_INC_PATH_OPTION) $(MAKE_CMD_XX_C) $< $(MAKE_CMD_OPTION_OUTPUT) $(_MAKE_OBJ_OUTPUT_PATH)/$(notdir $@)\n"\
"ifdef CC_RC\n"\
"ifneq ($(strip $(MAKE_RES_DLL_IMAGE_FILE)),)\n"\
"MAKE_CC_RC ?= $(strip $(CC_RC)) $(strip $(CC_RC_OPTION)) $(MAKE_INC_PATH_OPTION_PREFIX)$(basedir $<)) $(_MAKE_FULL_INC_PATH_OPTION) /rc $< /dllimg $(MAKE_RES_DLL_IMAGE_FILE) /objdir ${_MAKE_OBJ_OUTPUT_PATH}\n"\
"else\n"\
"MAKE_CC_RC ?= $(strip $(CC_RC)) $(strip $(CC_RC_OPTION)) $(MAKE_INC_PATH_OPTION_PREFIX)$(basedir $<) $(_MAKE_FULL_INC_PATH_OPTION) /fo\"$@\" $<\n"\
"endif\n"\
"endif\n"\
"\n"\
\
"MAKE_C_DIRS   :=$(rmdupword $(basedir $(MAKE_C_SRC_FILES)))\n"\
"MAKE_S_DIRS   :=$(rmdupword $(basedir $(MAKE_S_SRC_FILES)))\n"\
"MAKE_CPP_DIRS :=$(rmdupword $(basedir $(MAKE_CPP_SRC_FILES)))\n"\
"MAKE_RC_DIRS  :=$(rmdupword $(basedir $(MAKE_RC_SRC_FILES)))\n"\
"MAKE_XX_DIRS  :=$(rmdupword $(basedir $(MAKE_XX_SRC_FILES)))\n"\
"\n"\
"vpath %.c  $(MAKE_C_DIRS:= .:)\n"\
"vpath %.s  $(MAKE_S_DIRS:= .:)\n"\
"vpath %.cpp $(MAKE_CPP_DIRS:= .:)\n"\
"vpath %.$(MAKE_CC_RC_SRC_EXT_NAME) $(MAKE_RC_DIRS:= .:)\n"\
"vpath %.$(MAKE_CC_XX_SRS_EXT_NAME) $(MAKE_XX_DIRS:= .:)\n"\
"\n"\
\
"ifneq ($(strip $(MAKE_DISABLE_GCC_DEP_FEATURE)),YES)\n"\
"\n"\
\
"${MAKE_OBJ_OUTPUT_PATH}/%.d : %.c\n"\
"\t$(MAKE_DEP_CMD_LINE)\n"\
"\n"\
\
"${MAKE_OBJ_OUTPUT_PATH}/%.d : %.cpp\n"\
"\t$(MAKE_DEP_CMD_LINE)\n"\
"\n"\
\
"${MAKE_OBJ_OUTPUT_PATH}/%.d : %.s\n"\
"\t$(MAKE_DEP_CMD_LINE)\n"\
"\n"\
\
"${MAKE_OBJ_OUTPUT_PATH}/%.$(MAKE_CC_XX_OBJ_EXT_NAME) : %.$(MAKE_CC_XX_SRS_EXT_NAME)\n"\
"\t$(MAKE_DEP_CMD_LINE)\n"\
"\n"\
\
"endif # MAKE_DISABLE_GCC_DEP_FEATURE\n"\
\
"ifneq ($(strip $(MAKE_STD_SRC_FILE)), )\n"\
"${MAKE_STD_OBJ_FILE} : $(MAKE_STD_SRC_FILE)\n"\
"\t$(MAKE_CC_SRC_FILE_ECHO)\n"\
"\t$(MAKE_CC_STD)\n"\
"endif # MAKE_STD_SRC_FILE\n"\
"\n"\
\
"${MAKE_OBJ_OUTPUT_PATH}/%.$(MAKE_CC_OBJ_EXT_NAME) : %.c\n"\
"\t$(MAKE_CC_SRC_FILE_ECHO)\n"\
"\t$(MAKE_CC_C)\n"\
"\n"\
\
"${MAKE_OBJ_OUTPUT_PATH}/%.$(MAKE_CC_OBJ_EXT_NAME) : %.cpp\n"\
"\t$(MAKE_CC_SRC_FILE_ECHO)\n"\
"\t$(MAKE_CC_CPP)\n"\
"\n"\
\
"${MAKE_OBJ_OUTPUT_PATH}/%.o : %.s\n"\
"\t$(MAKE_CC_SRC_FILE_ECHO)\n"\
"\t$(MAKE_CC_S)\n"\
"\n"\
\
"ifdef MAKE_CC_RC\n"\
"${MAKE_OBJ_OUTPUT_PATH}/%.$(MAKE_CC_RC_OBJ_EXT_NAME) : %.$(MAKE_CC_RC_SRC_EXT_NAME)\n"\
"\t$(MAKE_CC_SRC_FILE_ECHO)\n"\
"\t$(MAKE_CC_RC)\n"\
"endif\n"\
"\n"\
\
"${MAKE_OBJ_OUTPUT_PATH}/%.$(MAKE_CC_XX_OBJ_EXT_NAME) : %.$(MAKE_CC_XX_SRS_EXT_NAME)\n"\
"\t$(MAKE_XX_SRC_FILE_ECHO)\n"\
"\t$(MAKE_XX_S)\n"\
"\n"\
"\n"\
\
"endif # MAKE_IMPLICIT_CC_RULES_TARGET\n"\
\
"\n"


//////////////////////////////////////////////////////////////////////////
// NO increment build(always compile source file)
// 
// MAKE_DEFAULT_NOINC_CC_FEATURES
//
// MAKE_C_SRC_FILES     like: c:/dir/1.c c:/dir/2.c
// MAKE_S_SRC_FILES
// MAKE_CPP_SRC_FILES
// MAKE_XX_SRC_FILES 
// CC_XX CC_XX_OPTION
// define target var : MAKE_IMPLICIT_FORCE_CC_RULES_TARGET
//////////////////////////////////////////////////////////////////////////
#define MAKE_IMPLICIT_FORCE_CC_RULES_LINES \
"MAKE_CC_SRC_FILE_ECHO ?=@print Compile \n"\
"MAKE_XX_SRC_FILE_ECHO ?=@print Compile \n"\
"MAKE_CC_RC_OBJ_EXT_NAME ?=res\n"\
"MAKE_CC_RC_SRC_EXT_NAME ?=rc\n"\
"MAKE_CC_XX_SRC_EXT_NAME ?=xxc\n"\
"MAKE_CC_XX_OBJ_EXT_NAME ?=xxo\n"\
\
"ifdef MAKE_DEFAULT_MSVC_FEATURES_SWT\n"\
"MAKE_CC_OBJ_EXT_NAME ?=obj\n"\
"MAKE_INC_PATH_OPTION_PREFIX ?=/I\n"\
"MAKE_CMD_OPTION_PREFIX ?=/\n"\
"MAKE_CMD_OPTION_OUTPUT ?=$(MAKE_CMD_OPTION_PREFIX)o\n"\
"else\n"\
"MAKE_CC_OBJ_EXT_NAME ?=o\n"\
"MAKE_INC_PATH_OPTION_PREFIX ?=-I\n"\
"MAKE_CMD_OPTION_PREFIX ?=-\n"\
"endif\n"\
"\n"\
"_MAKE_FULL_INC_PATH_OPTION =$(strip $(MAKE_FULL_INC_PATH_OPTION))\n"\
"_MAKE_OBJ_OUTPUT_PATH =$(strip $(MAKE_OBJ_OUTPUT_PATH))\n"\
\
"MAKE_CMD_S_C ?=$(MAKE_CMD_OPTION_PREFIX)c\n"\
"MAKE_CMD_S_INC_PATH_OPTION ?=$(_MAKE_FULL_INC_PATH_OPTION)\n"\
"MAKE_CMD_XX_C ?=$(MAKE_CMD_OPTION_PREFIX)c\n"\
"MAKE_CMD_XX_INC_PATH_OPTION ?=$(_MAKE_FULL_INC_PATH_OPTION)\n"\
"MAKE_CC_C   ?= $(strip $(CC)) $(strip $(CC_OPTION)) $(MAKE_INC_PATH_OPTION_PREFIX)$(basedir $(subst .$(MAKE_CC_OBJ_EXT_NAME),.c, $@)) $(_MAKE_FULL_INC_PATH_OPTION) $(MAKE_CMD_OPTION_PREFIX)c $(subst .$(MAKE_CC_OBJ_EXT_NAME),.c, $@) $(MAKE_CMD_OPTION_OUTPUT) $(_MAKE_OBJ_OUTPUT_PATH)/$(notdir $@) \n"\
"MAKE_CC_S   ?= $(strip $(CC_ASM)) $(strip $(CC_ASM_OPTION)) $(MAKE_INC_PATH_OPTION_PREFIX)$(basedir $(subst .$(MAKE_CC_OBJ_EXT_NAME),.s, $@)) $(MAKE_CMD_S_INC_PATH_OPTION) $(MAKE_CMD_S_C) $(subst .$(MAKE_CC_OBJ_EXT_NAME),.s, $@) $(MAKE_CMD_OPTION_OUTPUT) $(_MAKE_OBJ_OUTPUT_PATH)/$(notdir $@) \n"\
"MAKE_CC_CPP ?= $(strip $(CC_CPP)) $(strip $(CC_CPP_OPTION)) $(MAKE_INC_PATH_OPTION_PREFIX)$(basedir $(subst .$(MAKE_CC_OBJ_EXT_NAME),.cpp, $@)) $(_MAKE_FULL_INC_PATH_OPTION) $(MAKE_CMD_OPTION_PREFIX)c $(subst .$(MAKE_CC_OBJ_EXT_NAME),.cpp, $@) $(MAKE_CMD_OPTION_OUTPUT) $(_MAKE_OBJ_OUTPUT_PATH)/$(notdir $@) \n"\
"MAKE_CC_XX ?= $(strip $(CC_XX)) $(strip $(CC_XX_OPTION)) $(MAKE_INC_PATH_OPTION_PREFIX)$(basedir $(subst .$(MAKE_CC_XX_OBJ_EXT_NAME),.$(MAKE_CC_XX_SRC_EXT_NAME), $@)) $(MAKE_CMD_XX_INC_PATH_OPTION) $(MAKE_CMD_XX_C) $(subst .$(MAKE_CC_OBJ_EXT_NAME),.$(MAKE_CC_XX_SRC_EXT_NAME), $@) $(MAKE_CMD_OPTION_OUTPUT) $(_MAKE_OBJ_OUTPUT_PATH)/$(notdir $@) \n"\
\
"ifdef CC_RC\n"\
"ifneq ($(strip $(MAKE_RES_DLL_IMAGE_FILE)),)\n"\
"MAKE_CC_RC ?= $(strip $(CC_RC)) $(strip $(CC_RC_OPTION)) $(MAKE_INC_PATH_OPTION_PREFIX)$(basedir $(subst .$(MAKE_CC_RC_OBJ_EXT_NAME),.$(MAKE_CC_RC_SRC_EXT_NAME), $@)) $(_MAKE_FULL_INC_PATH_OPTION) /rc $(subst .$(MAKE_CC_RC_OBJ_EXT_NAME),.$(MAKE_CC_RC_SRC_EXT_NAME), $@) /dllimg $(MAKE_RES_DLL_IMAGE_FILE) /objdir ${_MAKE_OBJ_OUTPUT_PATH}\n"\
"else\n"\
"MAKE_CC_RC ?= $(strip $(CC_RC)) $(strip $(CC_RC_OPTION)) $(MAKE_INC_PATH_OPTION_PREFIX)$(basedir $(subst .$(MAKE_CC_RC_OBJ_EXT_NAME),.$(MAKE_CC_RC_SRC_EXT_NAME), $@)) $(_MAKE_FULL_INC_PATH_OPTION) /fo\"$(_MAKE_OBJ_OUTPUT_PATH)/$(notdir $@)\" $(subst .$(MAKE_CC_RC_OBJ_EXT_NAME),.$(MAKE_CC_RC_SRC_EXT_NAME), $@)\n"\
"endif\n"\
"endif\n"\
\
"\n"\
"MAKE_C_SRC_OBJ_FILES ?= $(subst .c,.$(MAKE_CC_OBJ_EXT_NAME), $(MAKE_C_SRC_FILES))\n"\
"MAKE_S_SRC_OBJ_FILES ?= $(subst .s,.$(MAKE_CC_OBJ_EXT_NAME), $(MAKE_S_SRC_FILES))\n"\
"MAKE_CPP_SRC_OBJ_FILES ?= $(subst .cpp,.$(MAKE_CC_OBJ_EXT_NAME), $(MAKE_CPP_SRC_FILES))\n"\
"MAKE_RC_SRC_OBJ_FILES ?= $(subst .$(MAKE_CC_RC_SRC_EXT_NAME),.$(MAKE_CC_RC_OBJ_EXT_NAME), $(MAKE_RC_SRC_FILES))\n"\
"MAKE_XX_SRC_OBJ_FILES ?= $(subst .$(MAKE_CC_XX_SRC_EXT_NAME),.$(MAKE_CC_XX_OBJ_EXT_NAME), $(MAKE_XX_SRC_FILES))\n"\
"MAKE_FULL_SRC_OBJ_FILES ?= ${foreach obj,$(MAKE_S_SRC_FILES) ${MAKE_C_SRC_FILES} ${MAKE_CPP_SRC_FILES} $(MAKE_XX_SRC_FILES) ${MAKE_USER_SRC_FILES},$(_MAKE_OBJ_OUTPUT_PATH)/$(basename $(notdir $(obj))).$(MAKE_CC_OBJ_EXT_NAME)}\n"\
"MAKE_FULL_SRC_RES_FILES ?= ${foreach obj,${MAKE_RC_SRC_FILES},$(_MAKE_OBJ_OUTPUT_PATH)/$(basename $(notdir $(obj))).$(MAKE_CC_RC_OBJ_EXT_NAME)}\n"\
\
"\n"\
"ifdef MAKE_IMPLICIT_FORCE_CC_RULES_TARGET\n" \
"$(MAKE_IMPLICIT_FORCE_CC_RULES_TARGET) : $(MAKE_S_SRC_OBJ_FILES) $(MAKE_C_SRC_OBJ_FILES) $(MAKE_CPP_SRC_OBJ_FILES) $(MAKE_RC_SRC_OBJ_FILES) $(MAKE_XX_SRC_OBJ_FILES) $(MAKE_FULL_USER_OBJ_FILES)\n"\
"endif\n"\
"\n"\
\
"ifneq ($(strip $(MAKE_C_SRC_OBJ_FILES)),)\n"\
"$(MAKE_C_SRC_OBJ_FILES) :\n"\
"\t$(MAKE_CC_SRC_FILE_ECHO) $(subst .$(MAKE_CC_OBJ_EXT_NAME),.c, $@)\n"\
"\t$(MAKE_CC_C)\n"\
"endif\n"\
\
"ifneq ($(strip $(MAKE_CPP_SRC_OBJ_FILES)),)\n"\
"$(MAKE_CPP_SRC_OBJ_FILES) :\n"\
"\t$(MAKE_CC_SRC_FILE_ECHO) $(subst .$(MAKE_CC_OBJ_EXT_NAME),.cpp, $@)\n"\
"\t$(MAKE_CC_CPP)\n"\
"endif\n"\
\
"ifneq ($(strip $(MAKE_S_SRC_OBJ_FILES)),)\n"\
"$(MAKE_S_SRC_OBJ_FILES) :\n"\
"\t$(MAKE_CC_SRC_FILE_ECHO) $(subst .$(MAKE_CC_OBJ_EXT_NAME),.s, $@)\n"\
"\t$(MAKE_CC_S)\n"\
"endif\n"\
\
"ifneq ($(strip $(MAKE_RC_SRC_OBJ_FILES)),)\n"\
"$(MAKE_RC_SRC_OBJ_FILES) :\n"\
"\t$(MAKE_CC_SRC_FILE_ECHO) $(subst .$(MAKE_CC_RC_OBJ_EXT_NAME),.$(MAKE_CC_RC_SRC_EXT_NAME), $@)\n"\
"\t$(MAKE_CC_RC)\n"\
"endif\n"\
"\n"\
\
"ifneq ($(strip $(MAKE_XX_SRC_OBJ_FILES)),)\n"\
"$(MAKE_XX_SRC_FILES) :\n"\
"\t$(MAKE_XX_SRC_FILE_ECHO) $(subst .$(MAKE_CC_XX_OBJ_EXT_NAME),.$(MAKE_CC_XX_SRC_EXT_NAME), $@)\n"\
"\t$(MAKE_CC_XX)\n"\
"endif\n"\
"\n"\
"\n"

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// .MAKE_FILE_LIST_MAKE_FEATURES
// 
//////////////////////////////////////////////////////////////////////////
#define MAKE_FILE_LIST_MAKE_LINES \
"__FILES_LIST_ := $(_PATH_FILE_LIST_CONFIG)\n"\
"_MAKIE_FILES_LIST_TARGET : $(foreach file,$(__FILES_LIST_),$(file)_FN_)\n"\
"$(foreach file,$(__FILES_LIST_),$(file)_FN_) :\n"\
"\n"

//////////////////////////////////////////////////////////////////////////
// .MAKE_MODULE_LIST_TARGETS
// 
//////////////////////////////////////////////////////////////////////////
#define MAKE_MODULE_LIST_LINES \
"MAKE_MULTI_TARGETS ?=_make_multi_targets\n"\
"ifneq ($(strip $(MAKE_MODULE_LIST)),)\n"\
"$(MAKE_MULTI_TARGETS) : $(MAKE_MODULE_LIST)\n"\
".PHONY: $(MAKE_MODULE_LIST)\n"\
"$(MAKE_MODULE_LIST):\n"\
"else\n"\
"$(MAKE_MULTI_TARGETS) :\n"\
"__dummy_$(MAKE_MULTI_TARGETS)__:\n"\
"endif\n"
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
#endif // _H
