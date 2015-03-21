#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gmediarender
PKG_VERSION:=2015-03-22
PKG_RELEASE=1.0

#PKG_SOURCE_PROTO:=git
#PKG_SOURCE_URL:=git@github.com:hzeller/gmrender-resurrect.git
#PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
#PKG_SOURCE_VERSION:=61f5a8fe7291995fec73956c7425fdb564e3cb9f
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_FIXUP:=autoreconf
PKG_INSTALL=1
PKG_REMOVE_FILES:=autogen.sh aclocal.m4

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/gmediarender
  SECTION:=multimedia
  CATEGORY:=Multimedia
  DEPENDS:= +gstreamer +glib2 +libupnp
  TITLE:=A Headless UPnP Renderer
endef

define Package/gmediarender/description
  (Yeso MODE)gmediarender implements the server component that provides UPnP
  controllers a means to render media content (audio, video and images)
  from a UPnP media server.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./gmediarender/* $(PKG_BUILD_DIR)/
endef

CONFIGURE_ARGS+= \
	--with-build-cc="$(HOSTCC)" \
	--prefix="\usr"

define Package/gmediarender/install
	$(INSTALL_DIR) 	$(1)/usr/sbin
	$(INSTALL_BIN) 	$(PKG_INSTALL_DIR)/usr/bin/$(PKG_NAME) $(1)/usr/sbin/
	$(INSTALL_DIR) 	$(1)/etc/init.d
	$(CP) 		$(PKG_BUILD_DIR)/scripts/init.d/$(PKG_NAME)er $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_DIR) 	$(1)/usr/share/gmediarender/
	$(CP) 		$(PKG_BUILD_DIR)/data/grender-64x64.png $(1)/usr/share/gmediarender/
	$(CP) 		$(PKG_BUILD_DIR)/data/grender-128x128.png $(1)/usr/share/gmediarender/
endef

$(eval $(call BuildPackage,gmediarender))
