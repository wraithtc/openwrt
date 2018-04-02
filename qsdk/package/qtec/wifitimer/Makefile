include $(TOPDIR)/rules.mk

PKG_NAME:=wifitimer
PKG_VERSION:=1.0
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/wifitimer
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=wifitimer
	DEPENDS:=libubus +libubox +libpthread +libuci
endef


define Package/wifitimer/description
	wifitimer.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/wifitimer/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifitimer $(1)/usr/bin
	$(INSTALL_DIR) $(1)/www/cgi-bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifitimer.cgi $(1)/www/cgi-bin
endef

$(eval $(call BuildPackage,wifitimer))