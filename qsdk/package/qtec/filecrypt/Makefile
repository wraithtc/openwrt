include $(TOPDIR)/rules.mk

PKG_NAME:=fscrypt
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/fscrypt
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=fscrypt
	DEPENDS:=+libfuse +libopenssl +spiinit
endef


define Package/fscrypt/description
        my fscrypt.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/fscrypt/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fscrypt $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/*.init $(1)/etc/init.d/fsinit
endef

$(eval $(call BuildPackage,fscrypt))
