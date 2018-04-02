include $(TOPDIR)/rules.mk

PKG_NAME:=wdsctrl
PKG_VERSION:=1.0
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/wdsctrl
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:= wdsctrl
	DEPENDS:= +libpthread +librtcfg 
endef


define Package/wdsctrl/description
	Display things on led screen.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wdsctrl $(1)/usr/bin
endef

define Package/wdsctrl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wdsctrl $(1)/usr/bin
endef

$(eval $(call BuildPackage,wdsctrl))
