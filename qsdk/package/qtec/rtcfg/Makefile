include $(TOPDIR)/rules.mk

PKG_NAME:=librtcfg
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/librtcfg
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=Custom librtcfg library
	DEPENDS:=+libuci +spiinit
endef

define Package/librtcfg/description
       librtcfg library
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib

define Build/InstallDev
	mkdir -p $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/*.h $(1)/usr/include/
	$(CP) -rf $(PKG_BUILD_DIR)/fwk/include/* $(1)/usr/include/
	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib*.so $(1)/usr/lib/
endef

define Package/librtcfg/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.so $(1)/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/smd $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wan_speedtest.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifi_txpower_init $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(CP) $(PKG_BUILD_DIR)/wifi_txpower.init $(1)/etc/init.d/wifi_txpower
	$(CP) $(PKG_BUILD_DIR)/guestWifiStartUp $(1)/etc/init.d/guestWifiStartUp
endef

$(eval $(call BuildPackage,librtcfg))
