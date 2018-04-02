
ifdef USER_DEPTH
  SRC_ROOT = $(USER_DEPTH)
  include $(SRC_ROOT)/config/makeinclude/macros.make
  include $(SRC_ROOT)/config/makeinclude/rules.make
else
  all clean: echo_error
endif # USER_DEPTH

echo_error:
	@echo "Error, Makefile must define USER_DEPTH."

