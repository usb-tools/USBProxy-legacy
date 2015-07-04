# Written by Alexander Holler
# Distributed under the terms of the GNU General Public License v2

# TODO: check for necessary kernel config options

EAPI="5"

EGIT_REPO_URI="git://github.com/dominicgs/USBProxy.git"

inherit cmake-utils git-r3

KEYWORDS="arm"

DESCRIPTION="A USB man in the middle device using the BeagleBone Black hardware"
HOMEPAGE="https://github.com/dominicgs/USBProxy"

LICENSE="GPL-2"
SLOT="0"

IUSE="lorcon pcap"

RDEPEND="lorcon? ( net-wireless/lorcon )
	pcap? ( net-libs/libpcap )
	virtual/libusb:1"
DEPEND="${RDEPEND}
	dev-util/cmake"

src_unpack() {
	[[ ${PV} == "9999" ]] && git-r3_src_unpack || default
}

CMAKE_USE_DIR="${S}/src"

src_configure() {
	local mycmakeargs=(
		$(cmake-utils_use_find_package lorcon lorcon)
		$(cmake-utils_use_find_package pcap PCAP)
	)
	cmake-utils_src_configure
}

src_compile() {
	cmake-utils_src_compile
}

src_install() {
    dodoc doc/inode.c.patch README.md doc/gadgetfs_kernel_above_3.15.patch
    newdoc doc/README.md README_kernel.md
	cmake-utils_src_install
}

pkg_postinst() {
        elog "You might have to use a patch for the kernel, read /usr/share/doc/${PF}/README_kernel.md"
}
