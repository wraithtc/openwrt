include $(TOPDIR)/rules.mk

PKG_NAME:=libqtkey
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/libqtkey
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=Custom libqtkey library
	DEPENDS:=
endef

define Package/libqtkey/description
       libqtkey library
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib

define Build/InstallDev
	mkdir -p $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/utilpack/h/*.h $(1)/usr/include/
	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/utilpack/*.a $(1)/usr/lib/
endef

define Package/libqtkey/install
	$(INSTALL_DIR) $(1)/lib
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/utilpack/*.a $(1)/lib
	mkdir -p $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/utilpack/h/*.h $(1)/usr/include/
endef

$(eval $(call BuildPackage,libqtkey))
