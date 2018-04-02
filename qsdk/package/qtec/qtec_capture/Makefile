#
# Copyright (C) 2013-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=qtec_capture
PKG_VERSION:=2018-1-3
PKG_RELEASE:=1.0.0

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)
PKG_MAINTAINER:=wjianjia
PKG_LICENSE:=GPL


include $(INCLUDE_DIR)/package.mk
#include $(INCLUDE_DIR)/cmake.mk

define Package/qtec_capture
  SECTION:=base
  CATEGORY:=Utilities
  TITLE:= qtec capture
  DEPENDS:=+libubox  +libuci  +librtcfg +libpcap +libopenssl
endef

define Package/qtec_capture/description
 This package provides one tool to capture lan client's system type  
endef


TARGET_CFLAGS += -ffunction-sections -fdata-sections -I$(STAGING_DIR)/usr/include
TARGET_LDFLAGS += -Wl,--gc-sections -L$(STAGING_DIR)/usr/lib
CMAKE_OPTIONS += $(if $(CONFIG_IPV6),,-DDISABLE_IPV6=1)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

#define Build/InstallDev
#	mkdir -p $(1)/usr/include
#	$(CP) $(PKG_BUILD_DIR)/*.h $(1)/usr/include/
#	mkdir -p $(1)/usr/lib
#	$(CP) $(PKG_BUILD_DIR)/*.so $(1)/usr/lib/
#endef

define Package/qtec_capture/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/qtec_capture $(1)/sbin/qtec_capture
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DATA) ./files/qtec_capture.config $(1)/etc/config/qtec_capture
endef

$(eval $(call BuildPackage,qtec_capture))
