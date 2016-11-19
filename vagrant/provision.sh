#!/bin/sh

[ -z "$CONFDIR" ] && CONFDIR=/etc/systemd/network/
[ -n "$CONFDIR" ] && mkdir -p $CONFDIR

# $1: interface name
# $2: network section
network_create() {
	local iface=$1
	local network=$2

	cat <<-EOF > "${CONFDIR}${iface}.network"
		[Match]
		Name=${iface}

		[Network]
		${network}
	EOF
}

# $1: interface name
# $2: kind [tun, tap]
netdev_create() {
	local iface=$1
	local kind=$2
	local section="$(tr '[:lower:]' '[:upper:]' <<< ${kind:0:1})${kind:1}"

	cat <<-EOF > "${CONFDIR}${iface}.netdev"
		[NetDev]
		Name=${iface}
		Kind=${kind}
	EOF

	case $kind in
		tun|tap)
			cat <<-EOF >> "${CONFDIR}${iface}.netdev"

				[$section]
				User=vagrant
			EOF
			;;
	esac
}

case $1 in
	libvirt)
		echo "libvirt detected"
		enslaved_eth=eth1
		;;
	parallels)
		echo "parallels detected"
		enslaved_eth=enp0s6
		;;
	virtualbox)
		echo "virtualbox detected" $CONFDIR
		enslaved_eth=enp0s8
		;;
	*)
		echo "Unkown provider: $1" 1>&2; exit 1
		;;
esac

netdev_create br0 bridge
netdev_create tap0 tap
network_create br0 DHCP=ipv4
network_create $enslaved_eth Bridge=br0
network_create tap0 Bridge=br0

# set grub timeout to 0 seconds
sed -i -e 's/.*GRUB_TIMEOUT.*/GRUB_TIMEOUT=0/g' /etc/default/grub
grub2-mkconfig -o /boot/grub2/grub.cfg

# disable selinux
sed -i -e 's/^SELINUX=.*/SELINUX=disabled/g' /etc/selinux/config
# use the fastest available mirror
if grep -q ".*fastestmirror.*=" /etc/dnf/dnf.conf; then
	sed -i -e 's/.*fastestmirror.*/fastestmirror=true/g' /etc/dnf/dnf.conf
else
	echo "fastestmirror=true" >> /etc/dnf/dnf.conf
fi

# Install development tools
dnf install -y make gcc

# set up a tap interface connected to a bridge
systemctl disable NetworkManager
systemctl enable systemd-networkd
systemctl enable systemd-resolved
rm -f /etc/resolv.conf
ln -s /run/systemd/resolve/resolv.conf /etc/resolv.conf

echo "Please reboot your vagrant box for the changes to take effect!" 1>&2
