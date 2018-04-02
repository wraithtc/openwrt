include $(TOPDIR)/rules.mk

PKG_NAME:=smbinit
PKG_VERSION:=1.0
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/smbinit
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:= smbinit
	DEPENDS:= +libpthread +spiinit +librtcfg
endef


define Package/smbinit/description
	init samba config.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/smbinit/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/smbinit $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/smb.init $(1)/etc/init.d/smbinit
endef

$(eval $(call BuildPackage,smbinit))
