include $(TOPDIR)/rules.mk

PKG_NAME:=qtec_firewall
PKG_VERSION:=1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/qtec_firewall
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:= qtec firewall collect and maintance firewall rule info upon uci 
	DEPENDS:=+libubox +libubus +libuci +librtcfg 
endef

define Package/qtec_firewall/description
	If you can't figure out what this program does, you're probably  
	brain-dead and need immediate medical attention.
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
	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib*.so $(1)/usr/lib/
endef

define Package/qtec_firewall/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.so $(1)/lib
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/qtec_firewall $(1)/bin/
endef

$(eval $(call BuildPackage,qtec_firewall))
