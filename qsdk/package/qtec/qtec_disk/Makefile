#
# Copyright (C) 2013-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=qtec_disk
PKG_VERSION:=2017-12-05
PKG_RELEASE:=1.0.0

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)
PKG_MAINTAINER:=wjianjia
PKG_LICENSE:=GPL


include $(INCLUDE_DIR)/package.mk
#include $(INCLUDE_DIR)/cmake.mk

define Package/qtec_disk
  SECTION:=base
  CATEGORY:=Utilities
  TITLE:= qtec disk
  DEPENDS:=+libubox  +libuci  +librtcfg +spiinit +libopenssl
endef

define Package/qtec_disk/description
 This package provides a qtec disk that handle disk-crypt 
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

define Package/qtec_disk/install
#	$(INSTALL_DIR) $(1)/lib
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.so $(1)/lib
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/qtec_getkey $(1)/sbin/qtec_getkey
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/qtec_diskmanager $(1)/sbin/qtec_diskmanager
	$(INSTALL_BIN) ./files/qtec_disk.sh $(1)/sbin/qtec_disk.sh
	$(INSTALL_BIN) ./files/qtec_disk_reformat.sh $(1)/sbin/qtec_disk_reformat.sh
	$(INSTALL_BIN) ./files/qtec_disk_check_and_init.sh $(1)/sbin/qtec_disk_check_and_init.sh
#	$(INSTALL_BIN) ./files/e2label $(1)/sbin/e2label
	$(INSTALL_DIR) $(1)/etc/hotplug.d/block
	$(INSTALL_DATA) ./files/qtec_disk.hotplug $(1)/etc/hotplug.d/block/21-qtec_disk
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DATA) ./files/qtec_disk.config $(1)/etc/config/qtec_disk
#	$(INSTALL_DIR) $(1)/etc/
#	$(INSTALL_DATA) ./files/qtec_disk.key $(1)/etc/qtec_disk.key
#	$(INSTALL_DATA) ./files/qtec_disk_pub.key $(1)/etc/qtec_disk_pub.key
#	$(INSTALL_DATA) ./files/qtec_disk.user $(1)/etc/qtec_disk.user
endef

$(eval $(call BuildPackage,qtec_disk))
