include $(TOPDIR)/rules.mk

PKG_NAME:=qtec_telnetd
PKG_VERSION:=1.0
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/qtec_telnetd
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:= qtec_telnetd
	DEPENDS:= +libpthread +librtcfg +qtec_cli
endef


define Package/qtec_telnetd/description
	cli lib.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ../qtec_cli/src/*.h $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/telnetd $(1)/usr/bin
endef

define Package/qtec_telnetd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/telnetd $(1)/usr/bin
#	$(INSTALL_DIR) $(1)/etc/init.d
#	$(CP) $(PKG_BUILD_DIR)/consoled.init $(1)/etc/init.d/qtec_consoled
endef

$(eval $(call BuildPackage,qtec_telnetd))
