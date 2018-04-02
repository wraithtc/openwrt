include $(TOPDIR)/rules.mk

PKG_NAME:=qtec_cli
PKG_VERSION:=1.0
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/qtec_cli
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:= qtec_cli
	DEPENDS:= +libpthread +librtcfg
endef


define Package/qtec_cli/description
	cli lib.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libqteccli.so $(1)/lib
endef

define Package/qtec_cli/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libqteccli.so $(1)/lib
endef

$(eval $(call BuildPackage,qtec_cli))
