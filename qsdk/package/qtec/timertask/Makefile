include $(TOPDIR)/rules.mk

PKG_NAME:=timertask
PKG_VERSION:=1.0
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/timertask
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=timertask
	DEPENDS:=libubus +libubox +libpthread +libuci +librtcfg +libstdcpp
endef


define Package/timertask/description
	timertask.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/timertask/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/timertask $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/timertask.init $(1)/etc/init.d/timertask
endef

$(eval $(call BuildPackage,timertask))