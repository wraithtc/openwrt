include $(TOPDIR)/rules.mk

PKG_NAME:=lanHostManager
PKG_VERSION:=1.0
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/lanHostManager
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=lanHostManager to collect lan host info
	DEPENDS:= +libubus +libubox +ubusd +libuci +libjson-c +libpthread +librtcfg +qtec_firewall
endef

define Package/lanHostManager/description
	the duty of lanHostManager is collect lan host info and return these info to other utilties.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/lanHostManager/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lanHostManager $(1)/bin/
endef

$(eval $(call BuildPackage,lanHostManager))
