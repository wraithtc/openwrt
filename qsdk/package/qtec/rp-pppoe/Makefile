#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk


PKG_NAME:=rp-pppoe
PKG_VERSION:=3.12
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.roaringpenguin.com/files/download/rp-pppoe-3.12.tar.gz
PKG_MD5SUM:=216eb52b69062b92a64ee37fd71f4b66

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

define Package/rp-pppoe
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=rp-pppoe
	DEPENDS:=+librtcfg +libpthread 
endef

define Package/rp-pppoe/description
	rp-pppoe
endef

define Package/rp-pppoe/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/pppoe-server $(1)/usr/sbin/
endef


$(eval $(call BuildPackage,rp-pppoe))