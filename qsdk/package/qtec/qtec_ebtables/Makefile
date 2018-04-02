#
# Copyright (C) 2013-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=qtec_ebtables
PKG_VERSION:=2017-09-30
PKG_RELEASE:=1.0.0

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)
PKG_MAINTAINER:=wjianjia
PKG_LICENSE:=GPL


include $(INCLUDE_DIR)/package.mk
#include $(INCLUDE_DIR)/cmake.mk

define Package/qtec_ebtables
  SECTION:=base
  CATEGORY:=Utilities
  TITLE:= qtec ebtables firewall
  DEPENDS:=+libubox  +libuci  +librtcfg
endef

define Package/qtec_ebtables/description
 This package provides a config-compatible C implementation of the UCI management to control ebtables rule.
endef


TARGET_CFLAGS += -ffunction-sections -fdata-sections -I$(STAGING_DIR)/usr/include
TARGET_LDFLAGS += -Wl,--gc-sections -L$(STAGING_DIR)/usr/lib
CMAKE_OPTIONS += $(if $(CONFIG_IPV6),,-DDISABLE_IPV6=1)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	mkdir -p $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/*.h $(1)/usr/include/
	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libqtecebtables.so $(1)/usr/lib/
endef

define Package/qtec_ebtables/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.so $(1)/lib
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/qtec_ebtables $(1)/sbin/qtec_ebtables
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/qtec_ebtables.init $(1)/etc/init.d/qtec_ebtables	
#	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
#	$(INSTALL_DATA) ./files/qtec_ebtables.hotplug $(1)/etc/hotplug.d/iface/21-qtec_ebtables
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DATA) ./files/qtec_ebtables.config $(1)/etc/config/qtec_ebtables
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_DATA) ./files/qtec_ebtables.user $(1)/etc/qtec_ebtables.user
endef

$(eval $(call BuildPackage,qtec_ebtables))
