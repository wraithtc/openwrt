include $(TOPDIR)/rules.mk

PKG_NAME:=qtec_tcpproxy
PKG_VERSION:=1.0
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/qtec_tcpproxy
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:= qtec_tcpproxy
	DEPENDS:= +libpthread +librtcfg +spiinit
endef


define Package/qtec_tcpproxy/description
	Display things on led screen.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tcpproxy $(1)/usr/bin
endef

define Package/qtec_tcpproxy/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tcpproxy $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/tcpproxy.init $(1)/etc/init.d/tcpproxy
endef

$(eval $(call BuildPackage,qtec_tcpproxy))
