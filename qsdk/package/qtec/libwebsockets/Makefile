include $(TOPDIR)/rules.mk

PKG_NAME:=libwebsockets
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS += -DLWS_OPENSSL_CLIENT_CERTS=/etc/ssl/certs
CMAKE_OPTIONS += -DLWS_OPENSSL_SUPPORT=ON
CMAKE_OPTIONS += -DLWS_WITH_SSL=ON
CMAKE_OPTIONS += -DLWS_WITHOUT_TESTAPPS=OFF

define Package/libwebsockets/Default
        SECTION:=libs
        CATEGORY:=Libraries
        TITLE:=libwebsockets
        DEPENDS:=+zlib +libopenssl +libpthread
endef

define Package/libwebsockets
        $(call Package/libwebsockets/Default)
        TITLE+= (qtec libraries)
endef

define Package/libwebsockets/description
       libwebsockets
       This package contains libwebsocket libraries
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib

define Build/InstallDev
	mkdir -p $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/lib/*.h $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/*.h $(1)/usr/include
	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib/libwebsockets.so* $(1)/usr/lib/
endef

define Package/libwebsockets/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_BUILD_DIR)/lib/libwebsockets.so* $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/lib/libwebsockets.h $(1)/usr/include
endef

$(eval $(call BuildPackage,libwebsockets))
