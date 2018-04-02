include $(TOPDIR)/rules.mk

PKG_NAME:=libqtkeymangent
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/libqtkeymangent
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=Custom libqtkeymangent library
	DEPENDS:=+libqtkey
endef


define Package/libqtkeymangent/description
       libqtkeymangent library
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
#	$(CP) ../QtKey_utilpack/src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib

define Build/InstallDev
mkdir -p $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/QtkeyMangent/include/*.h $(1)/usr/include/
#	$(CP) $(PKG_BUILD_DIR)/QtKeyDistribute/include/*.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/include/*.h $(1)/usr/include/
#	$(CP) $(PKG_BUILD_DIR)/utilpack/h/*.h $(1)/usr/include/
	$(CP) ../QtKey_utilpack/src/utilpack/h/*.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/QtKeyEncrypt/include/*.h $(1)/usr/include/
	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libs/lib*.so $(1)/usr/lib/
endef

define Package/libqtkeymangent/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libs/*.so $(1)/lib
endef

$(eval $(call BuildPackage,libqtkeymangent))
