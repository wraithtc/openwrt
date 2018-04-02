include $(TOPDIR)/rules.mk

PKG_NAME:=myclient
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/myclient
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=mywebsockets
	DEPENDS:=+librtcfg +libwebsockets +libpthread +libuci +libsqlite3 +jsoncgi +spiinit
endef


define Package/myclient/description
        my websocket client.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/myclient/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/myclient $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ca-certificates.crt $(1)/usr/bin 
endef

$(eval $(call BuildPackage,myclient))
