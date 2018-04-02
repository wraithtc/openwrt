include $(TOPDIR)/rules.mk

PKG_NAME:=onekeyswitch
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/onekeyswitch
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=onekeyswitch
	DEPENDS:=+librtcfg +libpthread
endef


define Package/onekeyswitch/description
        one key switch router
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/onekeyswitch/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/onekeyswitch $(1)/usr/sbin
endef

$(eval $(call BuildPackage,onekeyswitch))
