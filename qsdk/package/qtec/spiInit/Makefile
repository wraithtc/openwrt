include $(TOPDIR)/rules.mk

PKG_NAME:=spiinit
PKG_VERSION:=1.0
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/spiinit
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:= spiinit
	DEPENDS:= +libpthread 
endef


define Package/spiinit/description
	init spi device(security chip).
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	mkdir -p $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/sec_api.h $(1)/usr/include/
	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib*.so $(1)/usr/lib/
endef

define Package/spiinit/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/spiinit $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/spidev_test $(1)/usr/bin
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_BUILD_DIR)/*.so $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/sec_api.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/sec.init $(1)/etc/init.d/sec
endef

$(eval $(call BuildPackage,spiinit))
