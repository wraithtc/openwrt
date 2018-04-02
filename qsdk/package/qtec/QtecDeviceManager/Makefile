include $(TOPDIR)/rules.mk

PKG_NAME:=QtecDeviceManager
PKG_VERSION:=1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/QtecDeviceManager
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=QtecDeviceManager mantaining the user and device info 
	DEPENDS:= +libubus +libubox +ubusd +libuci +libjson-c +libpthread +libwebsockets +librtcfg +libsqlite3 +libqtkey
endef

define Package/QtecDeviceManager/description
	the duty of QtecDeviceManager can collect smart home device info and communicate to other utilties.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/QtecDeviceManager/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/QtecDeviceManager $(1)/bin/
	$(INSTALL_DIR) $(1)/sbin/
	$(INSTALL_BIN) ./files/qtec_check_zigbee.sh $(1)/sbin/
endef

$(eval $(call BuildPackage,QtecDeviceManager))
